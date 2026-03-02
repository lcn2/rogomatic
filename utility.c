/*
 * utility.c: Rog-O-Matic XIV (CMU) Tue Mar 26 15:27:03 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This file contains all of the miscellaneous system functions which
 * determine the baud rate, time of day, etc.
 *
 * If CMU is not defined, then various functions from libcmu.a are
 * defined here (otherwise the functions from -lcmu are used).
 */

# include <sgtty.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <signal.h>
# include <pwd.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <time.h>

# include "types.h"
# include "install.h"

# define TRUE 1
# define FALSE 0

/*
 * getname: get userid of player.
 */

char *
getname (void)
{ static char name[MU_BUF + 1]; /* +1 for paranoia */
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
{ int oldmask;
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
{ struct stat pbuf;

  return (stat (fn, &pbuf) == 0);
}

/*
 * filelength: Do a stat and return the length of a file.
 */

int
filelength (char *f)
{ struct stat sbuf;

  if (stat (f, &sbuf) == 0)
    return (sbuf.st_size);
  else
    return (-1);
}

/*
 * critical: Disable interrupts
 */

static void   (*hstat)(int), (*istat)(int), (*qstat)(int), (*pstat)(int);

void
critical (void)
{
  hstat = signal (SIGHUP, SIG_IGN);
  istat = signal (SIGINT, SIG_IGN);
  pstat = signal (SIGPIPE, SIG_IGN);
  qstat = signal (SIGQUIT, SIG_IGN);
}

/*
 * uncritical: Enable interrupts
 */

void
uncritical (void)
{
  signal (SIGHUP, hstat);
  signal (SIGINT, istat);
  signal (SIGPIPE, pstat);
  signal (SIGQUIT, qstat);
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
lock_file (char *lokfil, int maxtime)
{ int try;
  struct stat statbuf;

  start:
  if (creat (lokfil, NOWRITE) > 0)
    return TRUE;

  for (try = 0; try < 60; try++)
  { sleep (1);
    if (creat (lokfil, NOWRITE) > 0)
      return TRUE;
  }

  if (stat (lokfil, &statbuf) < 0)
  { creat (lokfil, NOWRITE);
    return TRUE;
  }

  if (time (0) - statbuf.st_mtime > maxtime)
  { if (unlink (lokfil) < 0)
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
unlock_file (char *lokfil)
{ unlink (lokfil);
}

# ifndef CMU
/*
 * quit: Defined for compatibility with Berkeley 4.2 system
 */

/* VARARGS2 */
int
quit (int code, char *fmt, ...)
{ va_list ap;
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
{ char *s, *b;
  s = small;
  b = big;
  do
  { if (*s == '\0')
      return (1);
  }
  while (*s++ == *b++);
  return (0);
}
# endif
