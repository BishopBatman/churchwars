/************************************************************************
 * configfile.c   Functions for dealing with dopewars config files      *
 * Copyright (C)  2002-2004  Ben Webb                                   *
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

#include <string.h>             /* For memcmp etc. */
#include <stdio.h>              /* For fgetc etc. */
#include <stdlib.h>             /* For atoi */
#include <errno.h>              /* For errno */
#include <sys/types.h>          /* For size_t etc. */
#include <sys/stat.h>
#include <ctype.h>              /* For isprint */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <glib.h>

#include "configfile.h"
#include "convert.h"            /* For Converter */
#include "dopewars.h"           /* For struct GLOBALS etc. */
#include "nls.h"                /* For _ function */
#include "error.h"              /* For ErrStrFromErrno */
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

static int mkdir_p(const char *path) {
    if (!path || !*path) return 0;
    char tmp[1024];
    size_t len = strlen(path);
    if (len >= sizeof tmp) return -1;
    strcpy(tmp, path);
    for (char *p = tmp + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0775) && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, 0775) && errno != EEXIST) return -1;
    return 0;
}

int ensure_scorefile_ready(const char *path) {
    if (!path || !*path) return -1;
    const char *slash = strrchr(path, '/');
    if (slash) {
        char dir[1024];
        size_t n = (size_t)(slash - path);
        if (n >= sizeof dir) return -1;
        memcpy(dir, path, n);
        dir[n] = '\0';
        if (mkdir_p(dir) != 0) return -1;
    }
    FILE *f = fopen(path, "ab+");
    if (!f) return -1;
    fclose(f);
    return 0;
}

gchar *LocalCfgEncoding = NULL;

#define MAX_CONFIG_FILE_SIZE (5 * 1024 * 1024)

/*
 * Prints the given string to a file, converting control characters
 * and escaping other special characters.
 */
static gboolean PrintEscaped(FILE *fp, gchar *str)
{
  guint i;
  guint len = strlen(str);

  for (i = 0; i < len; i++) {
    int ch = (int)(guchar)str[i];
    switch (ch) {
    case '"':
    case '\'':
    case '\\':
      if (fputc('\\', fp) == EOF || fputc(ch, fp) == EOF)
        return FALSE;
      break;
    case '\n':
      if (fputs("\\n", fp) == EOF)
        return FALSE;
      break;
    case '\t':
      if (fputs("\\t", fp) == EOF)
        return FALSE;
      break;
    case '\r':
      if (fputs("\\r", fp) == EOF)
        return FALSE;
      break;
    case '\b':
      if (fputs("\\b", fp) == EOF)
        return FALSE;
      break;
    case '\f':
      if (fputs("\\f", fp) == EOF)
        return FALSE;
      break;
    default:
      if (isascii(ch) && isprint(ch)) {
        if (fputc(ch, fp) == EOF)
          return FALSE;
      } else {
        if (fprintf(fp, "\\%o", ch) < 0)
          return FALSE;
      }
    }
  }

  return TRUE;
}

/*
 * Writes a single configuration file variable (identified by GlobalIndex
 * and StructIndex) to the specified file, in a format suitable for reading
 * back in (via. ParseNextConfig and friends).
 */
static gboolean WriteConfigValue(FILE *fp, Converter *conv, int GlobalIndex,
                                 int StructIndex)
{
  gchar *GlobalName;
  gboolean ok = TRUE;

  if (Globals[GlobalIndex].NameStruct[0]) {
    GlobalName =
        g_strdup_printf("%s[%d].%s", Globals[GlobalIndex].NameStruct,
                        StructIndex, Globals[GlobalIndex].Name);
  } else {
    GlobalName = Globals[GlobalIndex].Name;
  }

  if (Globals[GlobalIndex].IntVal) {
    if (fprintf(fp, "%s = %d\n", GlobalName,
                *GetGlobalInt(GlobalIndex, StructIndex)) < 0)
      ok = FALSE;
  } else if (Globals[GlobalIndex].BoolVal) {
    if (fprintf(fp, "%s = %s\n", GlobalName,
                *GetGlobalBoolean(GlobalIndex, StructIndex) ?
                "TRUE" : "FALSE") < 0)
      ok = FALSE;
  } else if (Globals[GlobalIndex].PriceVal) {
    gchar *prstr = pricetostr(*GetGlobalPrice(GlobalIndex, StructIndex));

    if (fprintf(fp, "%s = %s\n", GlobalName, prstr) < 0)
      ok = FALSE;
    g_free(prstr);
  } else if (Globals[GlobalIndex].StringVal) {
    gchar *convstr;

    if (fprintf(fp, "%s = \"", GlobalName) < 0)
      ok = FALSE;
    convstr = Conv_ToExternal(conv,
                              *GetGlobalString(GlobalIndex, StructIndex), -1);
    if (ok && !PrintEscaped(fp, convstr))
      ok = FALSE;
    g_free(convstr);
    if (ok && fputs("\"\n", fp) == EOF)
      ok = FALSE;
  } else if (Globals[GlobalIndex].StringList) {
    int i;
    gchar *convstr;

    if (fprintf(fp, "%s = { ", GlobalName) < 0)
      ok = FALSE;
    for (i = 0; ok && i < *Globals[GlobalIndex].MaxIndex; i++) {
      if (i > 0) {
        if (fputs(", ", fp) == EOF) {
          ok = FALSE;
          break;
        }
      }
      if (fputc('"', fp) == EOF) {
        ok = FALSE;
        break;
      }
      convstr = Conv_ToExternal(conv,
                                (*Globals[GlobalIndex].StringList)[i], -1);
      if (!PrintEscaped(fp, convstr))
        ok = FALSE;
      g_free(convstr);
      if (ok && fputc('"', fp) == EOF)
        ok = FALSE;
    }
    if (ok && fputs(" }\n", fp) == EOF)
      ok = FALSE;
  }

  if (Globals[GlobalIndex].NameStruct[0])
    g_free(GlobalName);

  if (!ok) {
    gchar *errstr = ErrStrFromErrno(errno);
    g_warning(_("Could not write to config file: %s"), errstr);
    g_free(errstr);
  }

  return ok;
}


static gboolean ReadFileToString(FILE *fp, gchar *str, int matchlen)
{
  int len, mpos, ch;
  gchar *match;
  GString *file;
  struct stat st;

  if (fstat(fileno(fp), &st) != 0) {
    gchar *errstr = ErrStrFromErrno(errno);
    g_warning(_("Could not stat config file: %s"), errstr);
    g_free(errstr);
    return FALSE;
  }

  if (st.st_size > MAX_CONFIG_FILE_SIZE) {
    g_warning(_("Config file too large"));
    return FALSE;
  }

  file = g_string_new("");
  len = strlen(str);
  if (matchlen > 0) {
    len = MIN(len, matchlen);
  }
  match = g_new(gchar, len);
  mpos = 0;

  while (mpos < len && file->len < MAX_CONFIG_FILE_SIZE &&
         (ch = fgetc(fp)) != EOF) {
    g_string_append_c(file, ch);
    match[mpos++] = ch;
    if (ch != str[mpos - 1]) {
      int start;
      gboolean shortmatch = FALSE;

      for (start = 1; start < mpos; start++) {
        if (memcmp(str, &match[start], mpos - start) == 0) {
          mpos -= start;
          memmove(match, &match[start], mpos);
          shortmatch = TRUE;
          break;
        }
      }
      if (!shortmatch)
        mpos = 0;
    }
  }

  if (file->len >= MAX_CONFIG_FILE_SIZE) {
    g_warning(_("Config file too large"));
    g_free(match);
    g_string_free(file, TRUE);
    return FALSE;
  }

  g_string_truncate(file, file->len - mpos);

  g_free(match);

  rewind(fp);
  if (ftruncate(fileno(fp), 0) != 0) {
    gchar *errstr = ErrStrFromErrno(errno);
    g_warning(_("Could not truncate config file: %s"), errstr);
    g_free(errstr);
    g_string_free(file, TRUE);
    return FALSE;
  }
  if (fputs(file->str, fp) == EOF) {
    gchar *errstr = ErrStrFromErrno(errno);
    g_warning(_("Could not write to config file: %s"), errstr);
    g_free(errstr);
    g_string_free(file, TRUE);
    return FALSE;
  }
  if (fputs(str, fp) == EOF) {
    gchar *errstr = ErrStrFromErrno(errno);
    g_warning(_("Could not write to config file: %s"), errstr);
    g_free(errstr);
    g_string_free(file, TRUE);
    return FALSE;
  }

  g_string_free(file, TRUE);
  return TRUE;
}

/*
 * Writes all of the configuration file variables that have changed
 * (together with their values) to the given file.
 */
static gboolean WriteConfigFile(FILE *fp, gboolean ForceUTF8)
{
  int i, j;
  Converter *conv = Conv_New();
  gboolean ok = TRUE;

  if (ForceUTF8 && !IsConfigFileUTF8()) {
    g_free(LocalCfgEncoding);
    LocalCfgEncoding = g_strdup("UTF-8");
    if (fputs("encoding \"UTF-8\"\n", fp) == EOF) {
      gchar *errstr = ErrStrFromErrno(errno);
      g_warning(_("Could not write to config file: %s"), errstr);
      g_free(errstr);
      Conv_Free(conv);
      return FALSE;
    }
  }

  if (LocalCfgEncoding && LocalCfgEncoding[0]) {
    Conv_SetCodeset(conv, LocalCfgEncoding);
  }

  for (i = 0; i < NUMGLOB && ok; i++) {
    if (Globals[i].Modified) {
      if (Globals[i].NameStruct[0]) {
        for (j = 1; ok && j <= *Globals[i].MaxIndex; j++) {
          if (!WriteConfigValue(fp, conv, i, j))
            ok = FALSE;
        }
      } else {
        if (!WriteConfigValue(fp, conv, i, 0))
          ok = FALSE;
      }
    }
  }
  Conv_Free(conv);
  return ok;
}

gboolean UpdateConfigFile(const gchar *cfgfile, gboolean ForceUTF8)
{
  FILE *fp;
  gchar *defaultfile;
  static gchar *header =
      "\n### Everything from here on is written automatically by\n"
      "### the Church Wars program; you can edit it manually, but any\n"
      "### formatting (comments, etc.) will be lost at the next rewrite.\n\n";

  defaultfile = GetLocalConfigFile();
  if (!cfgfile || !cfgfile[0]) {
    cfgfile = defaultfile;
    if (!cfgfile) {
      g_warning(_("Could not determine local config file to write to"));
      return FALSE;
    }
  }

  fp = fopen(cfgfile, "r+");
  if (!fp) {
    fp = fopen(cfgfile, "w+");
  }

  if (!fp) {
    gchar *errstr = ErrStrFromErrno(errno);
    g_warning(_("Could not open file %s: %s"), cfgfile, errstr);
    g_free(errstr);
    g_free(defaultfile);
    return FALSE;
  }

  if (!ReadFileToString(fp, header, 50)) {
    fclose(fp);
    g_free(defaultfile);
    return FALSE;
  }
  if (!WriteConfigFile(fp, ForceUTF8)) {
    fclose(fp);
    g_free(defaultfile);
    return FALSE;
  }

  fclose(fp);
  g_free(defaultfile);
  return TRUE;
}

gboolean IsConfigFileUTF8(void)
{
  return (LocalCfgEncoding && strcmp(LocalCfgEncoding, "UTF-8") == 0);
}
