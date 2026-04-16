/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>

#include "types.h"
#include "config.h"
#include "globals.h"

/* static declarations */
static FILE *debug = NULL;
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);

static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
  FILE *stream = stderr;  /* stream to write and flush */
  char buf[2*BIGBUF + 1]; /* +1 for paranoia */
  char *p;		  /* bug char to write write as "^" + un-controlled character */

  /* zeroize arrays */
  memset (buf, 0, sizeof(buf)); /* paranoia */

  /* form debug message */
  vsnprintf (buf, 2*BIGBUF, fmt, ap);

  /* divert output to debug stream if needed */
  if (debug != NULL) {
      stream = debug;
  }

  /*
   * process message a character at a time
   */
  for (p = buf; *p != '\0'; ++p) {

      /* special control character processing */
      if (iscntrl (*p)) {

	  switch (*p) {
	  case '\a':
	    fputs ("\\a", stream);
	    break;

	  case '\b':
	    fputs ("\\b", stream);
	    break;

	  case 0x1b: /* \e */
	    fputs ("^[", stream);		/* escape characters as ^[ */
	    break;

	  case '\f':
	    fputs ("\\f", stream);
	    break;

	  case '\n':
	    fputc ('\n', stream);		/* newlines show as newlines */
	    break;

	  case '\r':
	    fputs ("\\r", stream);
	    break;

	  case '\t':
	    fputs ("\\t", stream);
	    break;

	  case '\v':
	    fputs ("\\v", stream);
	    break;

	  /* non-special control character processing */
	  default:
	    fputc ('^', stream);		/* write out "^" prefix */
	    fputc (ctrl (*p) + 0100, stream);	/* un-control the control character */
	    break;
	  }

      /* as ^ (carrot) is special escape character, we escape them when they appear bare */
      } else if (*p == '^') {
	  fputs ("\\^", stream);

      /* as \\ (backslash) is special escape character, we escape them when they appear bare */
      } else if (*p == '\\') {
	  fputs ("\\\\", stream);

      /* DEL character, by convention, is denoted as ^? */
      } else if (*p == 0x7f) {
	  fputs ("^?", stream);

      /* non-control characters are written as they are */
      } else {
	  fputc (*p, stream);
      }
  }
  fflush (stream);
}

void
debuglog_open (const char *dir, const char *log)
{
  char *path = NULL; /* path to open */

  /* form full path */
  path = form_path (dir, log);

  /* open the log file */
  debug = fopen (path, "w");
  if (debug == NULL) {
    quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed to open for writing: %s: %s\n",
	      __func__, __FILE__, __LINE__, dnum, path, strerror (errno));
    not_reached ();
  }

  /* free memory */
  if (path != NULL) {
    free(path);
    path = NULL;
  }
}

void
debuglog_close (void)
{
  fclose (debug);
}

void
debuglog (const char *fmt, ...)
{
  va_list  ap;

  va_start (ap, fmt);
  err_doit (0, 0, fmt, ap);
  va_end (ap);
}

void
append_pidlog (const char *dir, const char *file)
{
  FILE *stream;		    /* pid log open stream */
  char *path;		    /* pid log path to open */
  int pidlog;		    /* pid log open file descriptor, or <0 ==> not open */
  struct timeval tp;	    /* now */
  struct tm *now;	    /* now broken out into segments */
  char timebuf[MU_BUF + 1]; /* now as a string */

  /*
   * form our time string
   */
  if (gettimeofday (&tp, NULL) < 0) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u gettimeofday failed: %s\n",
		     __func__, __FILE__, __LINE__, dnum, strerror(errno));
    exit (1);
  }
  now = gmtime (&tp.tv_sec);
  if (now == NULL) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u gmtime failed: %s\n",
		      __func__, __FILE__, __LINE__, dnum, strerror(errno));
    exit (1);
  }
  memset (timebuf, 0, sizeof(timebuf));
  if (strftime (timebuf, MU_BUF, "%F %T", now) <= 0) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u strftime failed: %s\n",
		     __func__, __FILE__, __LINE__, dnum, strerror(errno));
    exit (1);
  }

  /* form full path to a pid log */
  path = form_path (dir, file);

  /*
   * be sure that the pid log exists, writable at the end of file, and mode 0644 under umask
   *
   * Even though we will freopen(3) next, we want to first open(2) the error log file
   * as this will give us better control over the file if needs to be created.
   */
  pidlog = open (path, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (pidlog < 0) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Couldn't open pid log %s: %s\n",
		      __func__, __FILE__, __LINE__, dnum, path, strerror(errno));
    exit (1);
  }

  /*
   * convert pid log descriptor into a pid log stream
   */
  stream = fdopen (pidlog, "a");
  if (stream == NULL) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Couldn't freopen pid log: %s onto stderr: %s\n",
		     __func__, __FILE__, __LINE__, dnum, path, strerror(errno));
    exit (1);
  }

  /*
   * append line to pid log
   */
  fprintf (stream, "%ld.%06ld %s rogue-pid: %d player-pid: %d dungeon: %u\n",
		   (long) tp.tv_sec, (long) tp.tv_usec, timebuf, rogpid, (int) getpid(), dnum);

  /*
   * close pid log
   */
  fclose (stream);

  /* free memory */
  if (path != NULL) {
    free(path);
    path = NULL;
  }
}
