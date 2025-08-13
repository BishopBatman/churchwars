/************************************************************************
 * util.c         Miscellaneous utility and portability functions       *
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef CYGWIN
#include <conio.h>
#endif

#include <string.h>
#include "util.h"
#include "dopewars.h"

#ifndef HAVE_GETOPT
/* Global state used by the non-reentrant getopt() wrapper. */
char *optarg;
int optind = 1;
int optopt;
static struct getopt_state global_state = {NULL, 1, 0, 1};

/*
 * Reentrant getopt implementation. The parsing state is stored entirely in
 * the caller-provided structure "state" allowing multiple, concurrent uses
 * in different threads.
 */
int getopt_r(int argc, char *const argv[], const char *str,
             struct getopt_state *state)
{
  char *arg, *pt;
  char c;

  if (state->optind == 0) {
    state->optind = 1;
    state->apos = 1;
  }

  state->optarg = NULL;

  if (state->optind >= argc || argv[state->optind] == NULL) {
    state->apos = 1;
    return -1;
  }

  arg = argv[state->optind];
  if (arg[0] != '-' || arg[1] == '\0') {
    state->apos = 1;
    return -1;
  }
  if (strcmp(arg, "--") == 0) {
    state->optind++;
    state->apos = 1;
    return -1;
  }

  c = arg[state->apos++];
  state->optopt = c;
  pt = strchr(str, c);
  if (!pt) {
    if (arg[state->apos] == '\0') {
      state->optind++;
      state->apos = 1;
    }
    return '?';
  }

  if (*(pt + 1) == ':') {
    if (arg[state->apos] != '\0') {
      state->optarg = &arg[state->apos];
      state->optind++;
      state->apos = 1;
    } else if (state->optind + 1 < argc && argv[state->optind + 1]) {
      state->optarg = argv[++state->optind];
      state->optind++;
      state->apos = 1;
    } else {
      state->optind++;
      state->apos = 1;
      return (str[0] == ':') ? ':' : '?';
    }
  } else if (arg[state->apos] == '\0') {
    state->optind++;
    state->apos = 1;
  }

  return c;
}

/* Wrapper maintaining compatibility with traditional getopt(). The global
 * variables are updated after the reentrant function completes.  This
 * wrapper shares state and therefore should not be used concurrently from
 * multiple threads.
 */
int getopt(int argc, char *const argv[], const char *str)
{
  int ret;

  /* Copy global variables into our state before parsing in case the caller
   * has modified them (e.g. to reset optind to 0).
   */
  global_state.optarg = optarg;
  global_state.optind = optind;
  global_state.optopt = optopt;

  ret = getopt_r(argc, argv, str, &global_state);

  optarg = global_state.optarg;
  optind = global_state.optind;
  optopt = global_state.optopt;

  return ret;
}
#endif /* HAVE_GETOPT */


#ifdef CYGWIN                   /* Code for native Win32 build under Cygwin */

void sigemptyset(int *mask)
{
}

void sigaddset(int *mask, int sig)
{
}

int sigaction(int sig, struct sigaction *sact, char *pt)
{
  return 0;
}

void sigprocmask(int flag, int *mask, char *pt)
{
}

/*static gboolean IsKeyPressed()
{
  INPUT_RECORD ConsoleIn;
  DWORD NumConsoleIn;

  while (PeekConsoleInput(hIn, &ConsoleIn, 1, &NumConsoleIn)
         && NumConsoleIn == 1) {
    if (ConsoleIn.EventType == KEY_EVENT
        && ConsoleIn.Event.KeyEvent.bKeyDown) {
      return TRUE;
    } else {
      ReadConsoleInput(hIn, &ConsoleIn, 1, &NumConsoleIn);
    }
  }
  return FALSE;
}*/

int bselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
            struct timeval *tm)
{
  int retval;
  struct timeval tv, *tp;
  fd_set localread, localwrite, localexcept;
  char CheckKbHit = 0;

  if (nfds == 0 && tm) {
    Sleep(tm->tv_sec * 1000 + tm->tv_usec / 1000);
    return 0;
  }
  if (readfds && FD_ISSET(0, readfds)) {
    if (nfds == 1)
      return 1;
    tp = &tv;
    CheckKbHit = 1;
    FD_CLR(0, readfds);
  } else
    tp = tm;
  while (1) {
    tv.tv_sec = 0;
    tv.tv_usec = 250000;

    if (readfds)
      memcpy(&localread, readfds, sizeof(fd_set));
    if (writefds)
      memcpy(&localwrite, writefds, sizeof(fd_set));
    if (exceptfds)
      memcpy(&localexcept, exceptfds, sizeof(fd_set));
    if (CheckKbHit && kbhit())
      tv.tv_usec = 0;
    retval = select(nfds, readfds, writefds, exceptfds, tp);
    if (retval == SOCKET_ERROR)
      return retval;
    if (CheckKbHit && kbhit()) {
      retval++;
      if (readfds)
        FD_SET(0, readfds);
    }
    if (retval > 0 || !CheckKbHit)
      break;
    if (CheckKbHit && tm) {
      if (tm->tv_usec >= 250000)
        tm->tv_usec -= 250000;
      else if (tm->tv_sec) {
        tm->tv_usec += 750000;
        tm->tv_sec--;
      } else
        break;
    }
    if (readfds)
      memcpy(readfds, &localread, sizeof(fd_set));
    if (writefds)
      memcpy(writefds, &localwrite, sizeof(fd_set));
    if (exceptfds)
      memcpy(exceptfds, &localexcept, sizeof(fd_set));
  }
  return retval;
}

/* We don't do locking under Win32 right now */
int ReadLock(FILE * fp)
{
  return 0;
}

int WriteLock(FILE * fp)
{
  return 0;
}

void ReleaseLock(FILE * fp)
{
}

#else /* Code for Unix build */

#include <errno.h>

static int DoLock(FILE * fp, int l_type)
{
  struct flock lk;

  lk.l_type = l_type;
  lk.l_whence = lk.l_start = lk.l_len = 0;
  lk.l_pid = 0;

  do {
    if (fcntl(fileno(fp), F_SETLKW, &lk) == 0) {
      return 0;
    }
  } while (errno == EINTR);
  return 1;
}

int ReadLock(FILE * fp)
{
  return DoLock(fp, F_RDLCK);
}

int WriteLock(FILE * fp)
{
  return DoLock(fp, F_WRLCK);
}

void ReleaseLock(FILE * fp)
{
  (void)DoLock(fp, F_UNLCK);
}

#endif /* CYGWIN */

/* 
 * On systems with select, sleep for "microsec" microseconds.
 */
void MicroSleep(int microsec)
{
#if defined(HAVE_SELECT) || CYGWIN
  struct timeval tv;

  tv.tv_sec = 0;
  tv.tv_usec = microsec;
  bselect(0, NULL, NULL, NULL, &tv);
#endif
}
