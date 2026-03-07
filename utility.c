/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
 * Copyright (C) 1985 by Appel, Jacobson, Hamey, and Mauldin.
 *
 * This file is part of Rog-O-Matic.
 *
 * Rog-O-Matic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rog-O-Matic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rog-O-Matic.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * utility.c:
 *
 * This file contains all of the miscellaneous system functions which
 * determine the baud rate, time of day, etc.
 *
 * If CMU is not defined, then various functions from libcmu.a are
 * defined here (otherwise the functions from -lcmu are used).
 */

# include <curses.h>
# include <pwd.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <time.h>
# include <fcntl.h>
# include <unistd.h>
# include <signal.h>

# include "types.h"
# include "install.h"

# define TRUE 1
# define FALSE 0

/*
 * rogo_baudrate: Determine the baud rate of the terminal
 */

int
rogo_baudrate (void)
{
  return (baudrate());
}

/*
 * getname: get userid of player.
 */

char *
getname (void)
{
  static char name[MU_BUF + 1]; /* +1 for paranoia */
  struct passwd *pw;

  /*
   * lookup the password entry relating to the real user ID of the calling process
   */
  pw = getpwuid(getuid());

  /*
   * paranoia check
   *
   * Only use the username of the real user ID of the calling process,
   * if the username is a non-empty string
   * */
  memset(name, 0, sizeof(name)); /* paranoia */
  if (pw != NULL && pw->pw_name != NULL && pw->pw_name[0] != '\0') {
      strncpy(name, pw->pw_name, MU_BUF);
  } else {
      strncpy(name, "nobody", MU_BUF);
  }

  return (name);
}

/*
 * wopen: Open a file for world access.
 */

FILE *
wopen(char *fname, char *mode)
{
  int oldmask;
  FILE *newlog;

  oldmask = umask (0111);
  newlog = fopen (fname, mode);
  umask (oldmask);

  return (newlog);
}

/*
 * fexists: return a boolean if the named file exists
 */

int
fexists (char *fn)
{
  struct stat pbuf;

  return (stat (fn, &pbuf) == 0);
}

/*
 * filelength: Do a stat and return the length of a file.
 */

int
filelength (char *f)
{
  struct stat sbuf;

  if (stat (f, &sbuf) == 0)
    return (sbuf.st_size);
  else
    return (-1);
}

/*
 * critical: Disable interrupts
 */

#if 0 /* XXX - fix "bug errors" when signal is called in critical() and uncritical() - XXX */
static void  (*hstat)(int);
static void  (*istat)(int);
static void  (*lstat)(int);
static void  (*pstat)(int);
#endif

void
critical (void)
{
#if 0 /* XXX - fix "bug errors" when signal is called in critical() and uncritical() - XXX */
  hstat = signal (SIGHUP, SIG_IGN);
  istat = signal (SIGINT, SIG_IGN);
  pstat = signal (SIGPIPE, SIG_IGN);
  qstat = signal (SIGQUIT, SIG_IGN);
#endif
}

/*
 * uncritical: Enable interrupts
 */

void
uncritical (void)
{
#if 0 /* XXX - fix "bug errors" when signal is called in critical() and uncritical() - XXX */
  signal (SIGHUP, hstat);
  signal (SIGINT, istat);
  signal (SIGPIPE, pstat);
  signal (SIGQUIT, qstat);
#endif
}

/*
 * reset_int: Set all interrupts to default
 */

void
reset_int (void)
{
  signal (SIGHUP, SIG_DFL);
  signal (SIGINT, SIG_DFL);
  signal (SIGPIPE, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
}

/*
 * int_exit: Set up a function to call if we get an interrupt
 */

void
int_exit (void (*exitproc)(int))
{
  if (signal (SIGHUP, SIG_IGN) != SIG_IGN)  signal (SIGHUP, exitproc);

  if (signal (SIGINT, SIG_IGN) != SIG_IGN)  signal (SIGINT, exitproc);

  if (signal (SIGPIPE, SIG_IGN) != SIG_IGN) signal (SIGPIPE, exitproc);

  if (signal (SIGQUIT, SIG_IGN) != SIG_IGN) signal (SIGQUIT, exitproc);
}

/*
 * lock_file: lock a file for a maximum number of seconds.
 *            Based on the method used in Rogue 5.2.
 */

# define NOWRITE 0

int
lock_file (const char *lokfil, int maxtime)
{

  int try;

  struct stat statbuf;

start:

  if (creat (lokfil, NOWRITE) > 0)
    return TRUE;

  for (try = 0; try < 60; try++) {
          sleep (1);

          if (creat (lokfil, NOWRITE) > 0)
            return TRUE;
        }

  if (stat (lokfil, &statbuf) < 0) {
    creat (lokfil, NOWRITE);
    return TRUE;
  }

  if (time (NULL) - statbuf.st_mtime > maxtime) {
    if (unlink (lokfil) < 0)
      return FALSE;

    goto start;
  }
  else
    return FALSE;
}

/*
 * unlock_file: Unlock a lock file.
 */

void
unlock_file (const char *lokfil)
{
  unlink (lokfil);
}

# ifndef CMU
/*
 * quit: Defined for compatibility with Berkeley 4.2 system
 */

void
quit (int code, char *fmt, ...)
{
  va_list ap;

  /* setup stdarg */
  va_start (ap, fmt);

  vfprintf (stderr, fmt, ap);
  va_end (ap);
  exit (code);
}

/*
 * stlmatch  --  match leftmost part of string
 *
 *  Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 *  Returns 1 iff initial characters of big match small exactly;
 *  else 0.
 *
 *  HISTORY
 * 18-May-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX from Ken Greer's routine.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 */

int
stlmatch (char *big, char *small)
{
  char *s, *b;
  s = small;
  b = big;

  do {
    if (*s == '\0')
      return (1);
  }
  while (*s++ == *b++);

  return (0);
}
# endif
