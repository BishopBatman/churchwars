/************************************************************************
 * sound.c        dopewars sound system                                 *
 * Copyright (C)  1998-2022  Ben Webb                                   *
 *                Email: benwebb@users.sf.net                           *
 *                WWW: https://dopewars.sourceforge.io/                 *
 *                                                                      *
 * This program is free software; you can redistribute it and/or        *
 * modify it under the terms of the GNU General Public License          *
 * as published by the Free Software Foundation; either version 2       *
 * of the License, or (at your option) any later version.               *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, write to the Free Software          *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,               *
 *                   MA  02111-1307, USA.                               *
 ************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#ifdef PLUGINS
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#else
#include "plugins/sound_sdl.h"
#include "plugins/sound_esd.h"
#include "plugins/sound_pulseaudio.h"
#include "plugins/sound_winmm.h"
#ifdef HAVE_COCOA
SoundDriver *sound_cocoa_init(void);
#endif
#endif

#include "dopewars.h"
#include "log.h"
#include "nls.h"
#include "sound.h"

#define NOPLUGIN "none"

static SoundDriver *driver = NULL;
static GSList *driverlist = NULL;
typedef SoundDriver *(*InitFunc)(void);
/*
 * Sound is disabled by default until a driver is explicitly opened.
 */
static gboolean sound_enabled = FALSE;

gchar *GetPluginList(void)
{
  GSList *listpt;
  GString *plugins;
  gchar *retstr;

  plugins = g_string_new("\""NOPLUGIN"\"");
  for (listpt = driverlist; listpt; listpt = g_slist_next(listpt)) {
    SoundDriver *drivpt = (SoundDriver *)listpt->data;

    if (drivpt && drivpt->name) {
      g_string_append_printf(plugins, ", \"%s\"", drivpt->name);
    }
  }
  retstr = plugins->str;
  g_string_free(plugins, FALSE);
  return retstr;
}

static void AddPlugin(InitFunc ifunc, void *module)
{
  SoundDriver *newdriver = (*ifunc)();

  if (newdriver) {
    dopelog(5, 0, "%s sound plugin init OK", newdriver->name);
    newdriver->module = module;
    driverlist = g_slist_append(driverlist, newdriver);
  }
}

/*
 * Public wrappers for unit tests to register and query plugins without
 * exposing the internal static helpers.
 */
void SoundAddPlugin(SoundDriver *(*ifunc)(void), void *module)
{
  AddPlugin(ifunc, module);
}

#ifdef PLUGINS
static void OpenModule(const gchar *modname, const gchar *fullname)
{
  InitFunc ifunc;
  gint len = strlen(modname);
  void *soundmodule;

  if (len > 6 && strncmp(modname, "lib", 3) == 0
      && strcmp(modname + len - 3, ".so") == 0) {
    GString *funcname;

    soundmodule = dlopen(fullname, RTLD_NOW);
    if (!soundmodule) {
      /* Avoid calling dlerror() directly as it has been associated with
       * crashes on some platforms.  Use strerror on errno instead so we still
       * get a useful message without risking a segfault. */
      dopelog(3, 0, "dlopen of %s failed: %s", fullname, g_strerror(errno));
      return;
    }

    funcname = g_string_new(modname);
    g_string_truncate(funcname, len - 3);
    g_string_erase(funcname, 0, 3);
    g_string_append(funcname, "_init");
    ifunc = dlsym(soundmodule, funcname->str);
    if (ifunc) {
      AddPlugin(ifunc, soundmodule);
    } else {
      dopelog(3, 0, "dlsym (%s) failed: %s", funcname->str,
              g_strerror(errno));
      dlclose(soundmodule);
    }
    g_string_free(funcname, TRUE);
  }
}

static void ScanPluginDir(const gchar *dirname)
{
  DIR *dir;

  dir = opendir(dirname);
  if (dir) {
    struct dirent *fileinfo;
    GString *modname;

    modname = g_string_new("");
    do {
      fileinfo = readdir(dir);
      if (fileinfo) {
        g_string_assign(modname, dirname);
	g_string_append_c(modname, G_DIR_SEPARATOR);
	g_string_append(modname, fileinfo->d_name);
        OpenModule(fileinfo->d_name, modname->str);
      }
    } while (fileinfo);
    g_string_free(modname, TRUE);
    closedir(dir);
  } else {
    dopelog(5, 0, "Cannot open dir %s", dirname);
  }
}
#endif

void SoundInit(void)
{
#ifdef PLUGINS
  ScanPluginDir(PLUGINDIR);
  ScanPluginDir("src/plugins/.libs");
  ScanPluginDir("plugins/.libs");
#else
#ifdef HAVE_ESD
  AddPlugin(sound_esd_init, NULL);
#endif
#ifdef HAVE_PULSEAUDIO
  AddPlugin(sound_pulseaudio_init, NULL);
#endif
#ifdef HAVE_SDL_MIXER
  AddPlugin(sound_sdl_init, NULL);
#endif
#ifdef HAVE_WINMM
  AddPlugin(sound_winmm_init, NULL);
#endif
#ifdef HAVE_COCOA
  AddPlugin(sound_cocoa_init, NULL);
#endif
#endif
  driver = NULL;
}

static SoundDriver *GetPlugin(const gchar *drivername)
{
  GSList *listpt;

  for (listpt = driverlist; listpt; listpt = g_slist_next(listpt)) {
    SoundDriver *drivpt = (SoundDriver *)listpt->data;

    if (drivpt && drivpt->name
        && (!drivername || g_ascii_strcasecmp(drivpt->name, drivername) == 0)) {
      return drivpt;
    }
  }
  return NULL;
}

SoundDriver *SoundGetPlugin(const gchar *drivername)
{
  return GetPlugin(drivername);
}

static gboolean
TryLoadPlugin(const gchar *name)
{
  driver = GetPlugin(name);
  if (!driver) {
    if (name) {
      fprintf(stderr, "Sound plugin '%s' not found; falling back.\n", name);
    }
    return FALSE;
  }
  if (driver->open && !driver->open()) {
    fprintf(stderr, "Sound plugin '%s' failed to initialize; falling back.\n",
            name);
    driver = NULL;
    return FALSE;
  }
  fprintf(stderr, "Sound plugin '%s' selected\n", driver->name);
  sound_enabled = TRUE;
  return TRUE;
}

void SoundOpen(gchar *drivername)
{
  const gchar *envplug;
  sound_enabled = FALSE;

  envplug = g_getenv("CHURCHWARS_SOUND_PLUGIN");
  if (envplug && envplug[0] && TryLoadPlugin(envplug)) {
    return;
  }

  if (drivername && strcmp(drivername, NOPLUGIN) != 0 &&
      TryLoadPlugin(drivername)) {
    return;
  }

#ifdef _WIN32
  if (TryLoadPlugin("winmm")) {
    return;
  }
#else
  if (TryLoadPlugin("sdl")) {
    return;
  }
#endif

  driver = GetPlugin(NULL);
  if (driver) {
    TryLoadPlugin(driver->name);
  }
}

void SoundClose(void)
{
#ifdef PLUGINS
  GSList *listpt;
  SoundDriver *listdriv;
#endif

  if (driver && driver->close) {
    driver->close();
    driver = NULL;
  }
#ifdef PLUGINS
  for (listpt = driverlist; listpt; listpt = g_slist_next(listpt)) {
    listdriv = (SoundDriver *)listpt->data;
    if (listdriv && listdriv->module) {
      dlclose(listdriv->module);
    }
  }
#endif
  g_slist_free(driverlist);
  driverlist = NULL;
  sound_enabled = FALSE;
}

void SoundPlay(const gchar *snd)
{
  if (sound_enabled && driver && driver->play && snd && snd[0]) {
    driver->play(snd);
  }
}

gboolean SoundEnable(gboolean enable)
{
  if (enable && driver == NULL) {
    sound_enabled = FALSE;
    return FALSE;
  }

  sound_enabled = enable;
  return sound_enabled;
}

gboolean IsSoundEnabled(void)
{
  return sound_enabled;
}
