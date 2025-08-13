/************************************************************************
 * winmain.c      Startup code and support for the Win32 platform       *
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

#ifdef CYGWIN

#include <direct.h>
#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>

#include "dopewars.h"
#include "log.h"
#include "nls.h"
#include "tstring.h"
#include "util.h"
#include "AIPlayer.h"
#include "message.h"
#include "serverside.h"
#include "sound.h"
#include "winmain.h"

#ifdef CURSES_CLIENT
#include "curses_client/curses_client.h"
#endif

#ifdef GUI_CLIENT
#include "gui_client/gtk_client.h"
#endif

#ifdef GUI_SERVER
#include "gtkport/gtkport.h"
#endif

static void ServerLogMessage(const gchar *log_domain,
                             GLogLevelFlags log_level,
                             const gchar *message, gpointer user_data)
{
  GString *text;

  text = GetLogString(log_level, message);
  if (text) {
    g_print("%s\n", text->str);
    g_string_free(text, TRUE);
  }
}

static void ServerPrintFunc(const gchar *string)
{
  DWORD NumChar;

  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), string, strlen(string),
               &NumChar, NULL);
}

static void LogMessage(const gchar *log_domain, GLogLevelFlags log_level,
                       const gchar *message, gpointer user_data)
{
  MessageBox(NULL, message, "Error", MB_OK | MB_ICONSTOP);
}

static GString *TextOutput = NULL;

static void WindowPrintStart()
{
  TextOutput = g_string_new("");
}

static void WindowPrintFunc(const gchar *string)
{
  g_string_append(TextOutput, string);
}

static void WindowPrintEnd()
{
  MessageBox(NULL, TextOutput->str, "Church Wars",
             MB_OK | MB_ICONINFORMATION);
  g_string_free(TextOutput, TRUE);
  TextOutput = NULL;
}

static FILE *LogFile = NULL;

gchar *appdata_path = NULL;

static void GetAppDataPath()
{
  appdata_path = g_strdup_printf("%s/churchwars", g_get_user_config_dir());
  if (g_mkdir_with_parents(appdata_path, 0700) != 0) {
    g_warning("Could not create directory %s: %s", appdata_path,
              g_strerror(errno));
  }
}

static gboolean LogFileStart()
{
  char *logfile = g_strdup_printf("%s/churchwars-log.txt",
                                  appdata_path ? appdata_path : ".");
  LogFile = fopen(logfile, "w");
  if (!LogFile) {
    g_warning("Could not open log file '%s' for writing", logfile);
    g_free(logfile);
    return FALSE;
  }
  g_free(logfile);
  return TRUE;
}

static void LogFilePrintFunc(const gchar *string)
{
  if (LogFile) {
    fputs(string, LogFile);
    fflush(LogFile);
  }
}

static void LogFileEnd(void)
{
  if (LogFile)
    fclose(LogFile);
}

gchar *GetBinaryDir(void)
{
  gchar *filename = NULL, *lastslash;
  gint filelen = 80;
  DWORD retval;

  while (1) {
    filename = g_realloc(filename, filelen);
    filename[filelen - 1] = '\0';
    retval = GetModuleFileName(NULL, filename, filelen);

    if (retval == 0) {
      g_free(filename);
      filename = NULL;
      break;
    } else if (filename[filelen - 1]) {
      filelen *= 2;
    } else
      break;
  }

  if (filename) {
    lastslash = strrchr(filename, '\\');
    if (lastslash)
      *lastslash = '\0';
  }

  return filename;
}

#ifdef ENABLE_NLS
static gchar *GetWindowsLocale(void)
{
  LCID userID;
  WORD lang, sublang;
  static gchar langenv[30];     /* Should only be 5 chars, so
                                 * this'll be plenty */
  gchar *oldlang;

  langenv[0] = '\0';
  if (g_strlcpy(langenv, "LANG=", sizeof(langenv)) >= sizeof(langenv)) {
    return NULL;
  }

  oldlang = getenv("LANG");

  if (oldlang) {
    g_print("Started with LANG = %s\n", oldlang);
    return NULL;
  }

  userID = GetUserDefaultLCID();
  lang = PRIMARYLANGID(LANGIDFROMLCID(userID));
  sublang = SUBLANGID(LANGIDFROMLCID(userID));

  gsize res, len;
#define APPEND_LANG(str)                                                      \
  do {                                                                       \
    res = g_strlcat(langenv, (str), sizeof(langenv));                        \
    if (res >= sizeof(langenv))                                              \
      return NULL;                                                           \
  } while (0)

  switch (lang) {
  case LANG_ENGLISH:
    APPEND_LANG("en");
    if (sublang == SUBLANG_ENGLISH_UK) {
      APPEND_LANG("_GB");
    }
    break;
  case LANG_FRENCH:
    APPEND_LANG("fr");
    if (sublang == SUBLANG_FRENCH_CANADIAN) {
      APPEND_LANG("_CA");
    }
    break;
  case LANG_GERMAN:
    APPEND_LANG("de");
    break;
  case LANG_POLISH:
    APPEND_LANG("pl");
    break;
  case LANG_SPANISH:
    APPEND_LANG("es");
    if (sublang == SUBLANG_SPANISH) {
      APPEND_LANG("_ES");
    }
    break;
  case LANG_NORWEGIAN:
    if (sublang == SUBLANG_NORWEGIAN_NYNORSK) {
      APPEND_LANG("nn");
    } else {
      APPEND_LANG("no");
    }
    break;
  case LANG_PORTUGUESE:
    APPEND_LANG("pt");
    if (sublang == SUBLANG_PORTUGUESE_BRAZILIAN) {
      APPEND_LANG("_BR");
    }
    break;
  }
#undef APPEND_LANG

  len = strlen(langenv);
  if (len > 5 && len < sizeof(langenv)) {
    g_print("Using Windows language %s\n", langenv);
    return langenv;
  } else
    return NULL;
}
#endif /* ENABLE_NLS */

/*
 * Converts the passed lpszCmdParam WinMain-style command line into
 * Unix-style (argc, argv) parameters (returned).
 */
void get_cmdline(const LPSTR lpszCmdParam, int *argc, gchar ***argv)
{
  gchar **split;
  gchar *tmp_cmdline;

  tmp_cmdline = g_strdup_printf("%s %s", PACKAGE, lpszCmdParam);
  split = g_strsplit(tmp_cmdline, " ", 0);
  *argc = 0;
  while (split[*argc] && split[*argc][0]) {
    (*argc)++;
  }
  *argv = split;
  g_free(tmp_cmdline);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpszCmdParam, int nCmdShow)
{
  gchar **argv;
  int argc;
  gboolean is_service;
  gchar *modpath;
  struct CMDLINE *cmdline;

#ifdef ENABLE_NLS
  gchar *winlocale;
  const char *charset;
#endif

  /* Are we running as an NT service? */
  is_service = (lpszCmdParam && strncmp(lpszCmdParam, "-N", 2) == 0);

  if (is_service) {
    modpath = GetBinaryDir();
    SetCurrentDirectory(modpath);
    g_free(modpath);
  }

  GetAppDataPath();

  if (LogFileStart())
    g_set_print_handler(LogFilePrintFunc);

  g_log_set_handler(NULL, LogMask() | G_LOG_LEVEL_MESSAGE,
                    ServerLogMessage, NULL);

  if (!is_service) {
    g_log_set_handler(NULL, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL,
                      LogMessage, NULL);
  }
#ifdef ENABLE_NLS
  winlocale = GetWindowsLocale();
  if (winlocale)
    putenv(winlocale);

  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
  LocaleIsUTF8 = g_get_charset(&charset);
#endif

  /* Informational comment placed at the start of the Windows log file
     (this is used for messages printed during processing of the config
     files - under Unix these are just printed to stdout) */
  g_print(_("# This is the Church Wars startup log, containing any\n"
            "# informative messages resulting from configuration\n"
            "# file processing and the like.\n\n"));

  get_cmdline(lpszCmdParam, &argc, &argv);

  cmdline = GeneralStartup(argc, argv);
  if (cmdline->logfile) {
    AssignName(&Log.File, cmdline->logfile);
  }
  OpenLog();
  SoundInit();
  if (cmdline->version || cmdline->help) {
    WindowPrintStart();
    g_set_print_handler(WindowPrintFunc);
    HandleHelpTexts(cmdline->help);
    WindowPrintEnd();
#ifdef NETWORKING
  } else if (is_service) {
    InitNetwork();
    Network = Server = TRUE;
    win32_init(hInstance, hPrevInstance, "mainicon");
    ServiceMain(cmdline);
    StopNetworking();
#endif
  } else if (cmdline->convert) {
    WindowPrintStart();
    g_set_print_handler(WindowPrintFunc);
    ConvertHighScoreFile(cmdline->convertfile);
    WindowPrintEnd();
  } else {
    InitNetwork();
    if (cmdline->server) {
#ifdef NETWORKING
#ifdef GUI_SERVER
      Server = TRUE;
      g_log_set_handler(NULL, G_LOG_LEVEL_CRITICAL, LogMessage, NULL);
      win32_init(hInstance, hPrevInstance, "mainicon");
      GuiServerLoop(cmdline, FALSE);
#else
      AllocConsole();
      SetConsoleTitle(_("Church Wars server"));
      g_log_set_handler(NULL,
                        LogMask() | G_LOG_LEVEL_MESSAGE |
                        G_LOG_LEVEL_WARNING, ServerLogMessage, NULL);
      g_set_print_handler(ServerPrintFunc);
      newterm(NULL, NULL, NULL);
      ServerLoop(cmdline);
#endif /* GUI_SERVER */
#else
      WindowPrintStart();
      g_set_print_handler(WindowPrintFunc);
      g_print(_("This binary has been compiled without networking "
                "support, and thus cannot run\nin server mode. "
                "Recompile passing --enable-networking to the "
                "configure script.\n"));
      WindowPrintEnd();
#endif /* NETWORKING */
    } else if (cmdline->ai) {
      AllocConsole();

      /* Title of the Windows window used for AI player output */
      SetConsoleTitle(_("Church Wars AI"));

      g_log_set_handler(NULL,
                        LogMask() | G_LOG_LEVEL_MESSAGE |
                        G_LOG_LEVEL_WARNING, ServerLogMessage, NULL);
      g_set_print_handler(ServerPrintFunc);
      AIPlayerLoop(cmdline);
    } else {
      switch (cmdline->client) {
      case CLIENT_AUTO:
        if (!GtkLoop(hInstance, hPrevInstance, cmdline, TRUE)) {
          AllocConsole();
          SetConsoleTitle(_("Church Wars"));
          CursesLoop(cmdline);
        }
        break;
      case CLIENT_WINDOW:
        GtkLoop(hInstance, hPrevInstance, cmdline, FALSE);
        break;
      case CLIENT_CURSES:
        AllocConsole();
        SetConsoleTitle(_("Church Wars"));
        CursesLoop(cmdline);
        break;
      }
    }
#ifdef NETWORKING
    StopNetworking();
#endif
  }
  FreeCmdLine(cmdline);
  CloseLog();
  LogFileEnd();
  g_strfreev(argv);
  CloseHighScoreFile();
  g_free(PidFile);
  g_free(Log.File);
  SoundClose();
  return 0;
}

#endif /* CYGWIN */
