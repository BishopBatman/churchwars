/************************************************************************
 * sound_pulseaudio.c    dopewars sound system (PulseAudio driver)      *
 * Copyright (C)  1998-2024  Ben Webb                                   *
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

#ifdef HAVE_PULSEAUDIO
#include <stdio.h>
#include <string.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <glib.h>
#include "../nls.h"
#include "../sound.h"

#define MAXCACHE 6

struct SoundCache {
  gchar *name;
  pa_sample_spec spec;
  guint8 *data;
  size_t len;
} cache[MAXCACHE];

static int nextcache;

static gboolean LoadWav(const gchar *fname, pa_sample_spec *spec,
                        guint8 **data, size_t *len)
{
  guint8 hdr[44];
  FILE *f;
  guint32 datalen;

  f = fopen(fname, "rb");
  if (!f) {
    return FALSE;
  }
  if (fread(hdr, 1, sizeof(hdr), f) != sizeof(hdr)) {
    fclose(f);
    return FALSE;
  }
  if (memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) {
    fclose(f);
    return FALSE;
  }

  /* format information */
  spec->channels = hdr[22] | (hdr[23] << 8);
  spec->rate = hdr[24] | (hdr[25] << 8) | (hdr[26] << 16) | (hdr[27] << 24);
  guint16 bits = hdr[34] | (hdr[35] << 8);
  if (bits == 8) {
    spec->format = PA_SAMPLE_U8;
  } else if (bits == 16) {
    spec->format = PA_SAMPLE_S16LE;
  } else {
    fclose(f);
    return FALSE;
  }

  datalen = hdr[40] | (hdr[41] << 8) | (hdr[42] << 16) | (hdr[43] << 24);
  *data = g_malloc(datalen);
  if (fread(*data, 1, datalen, f) != datalen) {
    g_free(*data);
    fclose(f);
    return FALSE;
  }
  fclose(f);
  *len = datalen;
  return TRUE;
}

static gboolean SoundOpen_PulseAudio(void)
{
  int i;
  int error;
  pa_sample_spec spec = { PA_SAMPLE_U8, 44100, 1 };
  pa_simple *s;

  for (i = 0; i < MAXCACHE; i++) {
    cache[i].name = NULL;
    cache[i].data = NULL;
    cache[i].len = 0;
  }
  nextcache = 0;

  s = pa_simple_new(NULL, PACKAGE, PA_STREAM_PLAYBACK, NULL,
                    "open", &spec, NULL, NULL, &error);
  if (!s) {
    g_warning(_("Cannot connect to PulseAudio server: %s"),
              pa_strerror(error));
    return FALSE;
  }
  pa_simple_free(s);
  return TRUE;
}

static void SoundClose_PulseAudio(void)
{
  int i;

  for (i = 0; i < MAXCACHE; i++) {
    g_free(cache[i].name);
    g_free(cache[i].data);
  }
}

static void SoundPlay_PulseAudio(const gchar *snd)
{
  int i, error;
  pa_simple *s;
  struct SoundCache *entry = NULL;

  for (i = 0; i < MAXCACHE; i++) {
    if (cache[i].name && strcmp(cache[i].name, snd) == 0) {
      entry = &cache[i];
      break;
    }
  }

  if (!entry) {
    pa_sample_spec spec;
    guint8 *data;
    size_t len;

    if (!LoadWav(snd, &spec, &data, &len)) {
      return;
    }
    if (cache[nextcache].name) {
      g_free(cache[nextcache].name);
      g_free(cache[nextcache].data);
    }
    cache[nextcache].name = g_strdup(snd);
    cache[nextcache].spec = spec;
    cache[nextcache].data = data;
    cache[nextcache].len = len;
    entry = &cache[nextcache];
    nextcache = (nextcache + 1) % MAXCACHE;
  }

  s = pa_simple_new(NULL, PACKAGE, PA_STREAM_PLAYBACK, NULL,
                    snd, &entry->spec, NULL, NULL, &error);
  if (!s) {
    g_warning("pa_simple_new failed: %s", pa_strerror(error));
    return;
  }
  if (pa_simple_write(s, entry->data, entry->len, &error) < 0) {
    g_warning("pa_simple_write failed: %s", pa_strerror(error));
    pa_simple_free(s);
    return;
  }
  if (pa_simple_drain(s, &error) < 0) {
    g_warning("pa_simple_drain failed: %s", pa_strerror(error));
  }
  pa_simple_free(s);
}

SoundDriver *sound_pulseaudio_init(void)
{
  static SoundDriver driver;

  driver.name = "pulseaudio";
  driver.open = SoundOpen_PulseAudio;
  driver.close = SoundClose_PulseAudio;
  driver.play = SoundPlay_PulseAudio;
  return &driver;
}

#endif /* HAVE_PULSEAUDIO */
