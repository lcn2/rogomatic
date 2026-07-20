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
#include <setjmp.h>

#include "types.h"
#include "config.h"
#include "globals.h"

/* static declarations */
static FILE *debug = NULL;
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);
static FILE *level_log_stream = NULL;		/* open stream to append to the rogomatic level log */
static FILE *total_level_log_stream = NULL;	/* open stream to append to the rogomatic total level log */

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
   * Even though we will fdopen(3) next, we want to first open(2) the error log file
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
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Couldn't fdopen pid log: %s onto stderr: %s\n",
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

/*
 * levellog_create - create and a new level log file, and open total level log file for appending
 */
void
levellog_create (void)
{
  int levellog_fd;	            /* level log open file descriptor, or <0 ==> not open */
  int total_levellog_fd;            /* total level log open file descriptor, or <0 ==> not open */

  /*
   * be sure that a new level log exists, writable at the beginning of file, and mode 0644 under umask
   *
   * Even though we will fdopen(3) next, we want to first open(2) the error log file
   * as this will give us better control over the file if needs to be created.
   */
  levellog_fd = open (level_path, O_WRONLY | O_APPEND | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (levellog_fd < 0) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Couldn't open level log %s: %s\n",
		     __func__, __FILE__, __LINE__, dnum, level_path, strerror(errno));
    exit (1);
  }

  /*
   * convert level log descriptor into a level log stream
   */
  level_log_stream = fdopen (levellog_fd, "a");
  if (level_log_stream == NULL) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Couldn't fdopen level log: %s: %s\n",
		     __func__, __FILE__, __LINE__, dnum, level_path, strerror(errno));
    exit (1);
  }

  /*
   * be sure that the total level log exists, writable at the beginning of file, and mode 0644 under umask
   *
   * Even though we will fdopen(3) next, we want to first open(2) the error log file
   * as this will give us better control over the file if needs to be created.
   */
  total_levellog_fd = open (total_level_path, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (total_levellog_fd < 0) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Couldn't open total level log %s: %s\n",
		     __func__, __FILE__, __LINE__, dnum, level_path, strerror(errno));
    exit (1);
  }

  /*
   * convert total level log descriptor into a total level log stream
   */
  total_level_log_stream = fdopen (total_levellog_fd, "a");
  if (total_level_log_stream == NULL) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Couldn't fdopen total level log: %s: %s\n",
		     __func__, __FILE__, __LINE__, dnum, total_level_path, strerror(errno));
    exit (1);
  }
  return;
}

/*
 * levellog_append - append state to the open level log file, and the total level log file
 */
void
levellog_append (char *reason)
{
  struct timeval tp;		    /* now */
  struct tm *now;		    /* now broken out into segments */
  char timebuf[MU_BUF + 1];	    /* now as a string */
  static int prev_lvl_no_reason = -1;	    /* if >= 0, previously logged level in lvllog, "with no reason" */
  static int prev_lvl_a_reason = -1;	    /* if >= 0, previously logged level in lvllog, "with a reason" */

  /*
   * firewall - level_log_stream and the total_level_log_stream must be open
   */
  if (level_log_stream == NULL) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u level log %s: level_log_stream is NULL\n",
		     __func__, __FILE__, __LINE__, dnum, level_path);
    exit (1);
  }
  if (total_level_log_stream == NULL) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u level log %s: total_level_log_stream is NULL\n",
		     __func__, __FILE__, __LINE__, dnum, total_level_path);
    exit (1);
  }

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

  /*
   * prevent too many successive logs for the same dungeon level
   *
   * If this function is called more than once for the same dungeon level,
   * we skip logging the same level.  The unstuck_player script watches the lvllog file
   * in an effort to try and detect a CPU bound loop.  In rare(?) cases, a CPU loop
   * might cause this function to be called multiple times for the same dungeon level.
   * So we prevent writing to the level log in succession for the same dungeon level.
   *
   * However .. there is an allowed case for a successive logs for the same dungeon level:
   * normally when exiting, the reason goes from NULL or an empty string ("with no reason")
   * to having a reason string ("with a reason").
   *
   * Therefore:
   *
   * we prevent logging successive logs for the same dungeon level "with no reason",
   * AND
   * we prevent logging successive logs for the same dungeon level "with a reason".
   */
  if (reason == NULL || reason[0] == '\0') {
    if (prev_lvl_no_reason < 0) {

      /* first call to this function "with no reason" */
      prev_lvl_no_reason = Level;

    } else if (prev_lvl_no_reason == Level) {

      /* don't log a successive call with same level "with no reason" */
      return;

    }
  } else {
    if (prev_lvl_a_reason < 0) {

      /* first call to this function "with a reason" */
      prev_lvl_a_reason = Level;

    } else if (prev_lvl_a_reason == Level) {

      /* don't log a successive call with same level "with a reason" */
      return;

    }
  }

  /*
   * append line to level log, and the total level log
   */
  if (reason == NULL || reason[0] == '\0') {
    /* level log "with no reason" */
    fprintf (level_log_stream, "%ld.%06ld %s Level: %d Gold: %d Hp: %d(%d) Str: %d(%d) Ac: %d Exp: %d/%d Am: %c ...\n",
			       (long) tp.tv_sec, (long) tp.tv_usec, timebuf,
			       Level, Gold, Hp, Hpmax, Str/100, Strmax/100, Ac, Explev, Exp,
			       ((have (amulet) != NONE) ? ',' : '-'));
    fprintf (total_level_log_stream, "%ld.%06ld %s Level: %d Gold: %d Hp: %d(%d) Str: %d(%d) Ac: %d Exp: %d/%d Am: %c ...\n",
			       (long) tp.tv_sec, (long) tp.tv_usec, timebuf,
			       Level, Gold, Hp, Hpmax, Str/100, Strmax/100, Ac, Explev, Exp,
			       ((have (amulet) != NONE) ? ',' : '-'));
  } else {
    /* level log "with a reason" */
    fprintf (level_log_stream, "%ld.%06ld %s Level: %d Gold: %d Hp: %d(%d) Str: %d(%d) Ac: %d Exp: %d/%d Am: %c %s\n",
			       (long) tp.tv_sec, (long) tp.tv_usec, timebuf,
			       Level, Gold, Hp, Hpmax, Str/100, Strmax/100, Ac, Explev, Exp,
			       ((have (amulet) != NONE) ? ',' : '-'), reason);
    fprintf (total_level_log_stream, "%ld.%06ld %s Level: %d Gold: %d Hp: %d(%d) Str: %d(%d) Ac: %d Exp: %d/%d Am: %c %s\n",
			       (long) tp.tv_sec, (long) tp.tv_usec, timebuf,
			       Level, Gold, Hp, Hpmax, Str/100, Strmax/100, Ac, Explev, Exp,
			       ((have (amulet) != NONE) ? ',' : '-'), reason);
  }
  fflush(level_log_stream);
  fflush(total_level_log_stream);

  /*
   * prep for the next log level call
   */
  if (reason == NULL || reason[0] == '\0') {

    /* remember this level, "with no reason", for the next call */
    prev_lvl_no_reason = Level;

  } else {

    /* remember this level, "with a reason", for the next call */
    prev_lvl_a_reason = Level;

  }
  return;
}
