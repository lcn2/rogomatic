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

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <pwd.h>
# include <signal.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <time.h>
# include <fcntl.h>
# include <errno.h>
# include <sys/file.h>
# include <termios.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "modern_curses.h"
# include "types.h"
# include "install.h"

# define TRUE 1
# define FALSE 0

static bool final_newline = false;	/* True is endwin_and_ncurses_cleanup() has been called */

static void  (*hstat)(int) = NULL;
static void  (*istat)(int) = NULL;
static void  (*pstat)(int) = NULL;
static void  (*qstat)(int) = NULL;
static void  (*tstat)(int) = NULL;

static char *termattr_path = NULL;

/*
 * save_termattr: save stdin, stdout, and stderr terminal state into saved_termattr file
 */

void
save_termattr (char *dir)
{
  struct termios tattr_stdin;    /* stdin terminal state */
  struct termios tattr_stdout;   /* stdout terminal state */
  struct termios tattr_stderr;   /* stderr terminal state */
  int f;			 /* saved_termattr open file descriptor */
  int ret;			 /* tcgetattr() return status */
  ssize_t wr_ret;		 /* write() return status */
  ssize_t cl_ret;		 /* close() return status */

  /*
   * obtain the stdin, stdout and stderr terminal states
   */
  memset (&tattr_stdin, 0, sizeof(tattr_stdin)); /* paranoia */
  ret = tcgetattr (STDIN_FILENO, &tattr_stdin);
  if (ret != 0) {
    fprintf(stderr, "Warning: terminal attributes not saved: tcgetattr of stdin state failed: %s", strerror (errno));
    return;
  }
  /**/
  memset (&tattr_stdout, 0, sizeof(tattr_stdout)); /* paranoia */
  ret = tcgetattr (STDOUT_FILENO, &tattr_stdout);
  if (ret != 0) {
    fprintf(stderr, "Warning: terminal attributes not saved: tcgetattr of stdout state failed: %s", strerror (errno));
    return;
  }
  /**/
  memset (&tattr_stderr, 0, sizeof(tattr_stderr)); /* paranoia */
  ret = tcgetattr (STDERR_FILENO, &tattr_stderr);
  if (ret != 0) {
    fprintf(stderr, "Warning: terminal attributes not saved: tcgetattr of stderr state failed: %s", strerror (errno));
    return;
  }

  /*
   * form saved_termattr path
   */
  termattr_path = form_path(dir, "saved_termattr");

  /*
   * open for writing (create if needed with more 0644) saved_termattr
   */
  f = open (termattr_path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); /* mode 0644 */
  if (f < 0) {

    /* indicate that saved_termattr is not setup */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: open: %s failed: %s\n", termattr_path, strerror (errno));
    return;

  }

  /*
   * write stdin, stdout and stderr terminal states to the saved_termattr file
   */
  wr_ret = write (f, &tattr_stdin, sizeof(tattr_stdin));
  if (wr_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: write of stdin state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  } else if (wr_ret != sizeof(tattr_stdin)) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: write size of stdin state: %zd != %zu", wr_ret, sizeof(tattr_stdin));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }
  /**/
  wr_ret = write (f, &tattr_stdout, sizeof(tattr_stdout));
  if (wr_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: write of stdout state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  } else if (wr_ret != sizeof(tattr_stdout)) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: write size of stdout state: %zd != %zu", wr_ret, sizeof(tattr_stdout));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }
  /**/
  wr_ret = write (f, &tattr_stderr, sizeof(tattr_stderr));
  if (wr_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: write of stderr state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  } else if (wr_ret != sizeof(tattr_stderr)) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: write size of stderr state: %zd != %zu", wr_ret, sizeof(tattr_stderr));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }

  /*
   * close saved_termattr
   */
  cl_ret = close(f);
  if (cl_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));

  }
  return;
}

/*
 * restore_termattr: restore terminal state for stdin, stdout, and stderr from the saved_termattr file
 */

void
restore_termattr (void)
{
  struct termios tattr_stdin;    /* stdin terminal state */
  struct termios tattr_stdout;   /* stdout terminal state */
  struct termios tattr_stderr;   /* stderr terminal state */
  int f;			 /* saved_termattr open file descriptor */
  int ret;			 /* tcsetattr() return status */
  ssize_t rd_ret;		 /* read() return status */
  ssize_t cl_ret;		 /* close() return status */

  /*
   * firewall
   */
  if (termattr_path == NULL) {
    fprintf(stderr, "Warning: terminal attributes not restored: saved_termattr file was not formed, or was incomplete\n");
    return;
  }

  /*
   * open of reading saved_termattr
   */
  f = open (termattr_path, O_RDONLY);
  if (f < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: open: %s failed: %s\n", termattr_path, strerror (errno));
    return;

  }

  /*
   * read stdin, stdout and stderr terminal states from the saved_termattr file
   */
  memset (&tattr_stdin, 0, sizeof(tattr_stdin)); /* paranoia */
  rd_ret = read (f, &tattr_stdin, sizeof(tattr_stdin));
  if (rd_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: read of stdin state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  } else if (rd_ret != sizeof(tattr_stdin)) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: read size of stdin state: %zd != %zu", rd_ret, sizeof(tattr_stdin));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }
  /**/
  memset (&tattr_stdout, 0, sizeof(tattr_stdout)); /* paranoia */
  rd_ret = read (f, &tattr_stdout, sizeof(tattr_stdout));
  if (rd_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: read of stdout state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  } else if (rd_ret != sizeof(tattr_stdout)) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: read size of stdout state: %zd != %zu", rd_ret, sizeof(tattr_stdout));
    return;

  }
  /**/
  memset (&tattr_stderr, 0, sizeof(tattr_stderr)); /* paranoia */
  rd_ret = read (f, &tattr_stderr, sizeof(tattr_stderr));
  if (rd_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: read of stderr state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  } else if (rd_ret != sizeof(tattr_stderr)) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: read size of stderr state: %zd != %zu", rd_ret, sizeof(tattr_stderr));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }

  /*
   * restore the stdin, stdout and stderr terminal states
   */
  ret = tcsetattr (STDIN_FILENO, TCSANOW, &tattr_stdin);
  if (ret != 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: tcsetattr of stdin state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }
  /**/
  ret = tcsetattr (STDOUT_FILENO, TCSANOW, &tattr_stdout);
  if (ret != 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: tcsetattr of stdout state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }
  /**/
  ret = tcsetattr (STDERR_FILENO, TCSANOW, &tattr_stderr);
  if (ret != 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not restored: tcsetattr of stderr state failed: %s", strerror (errno));

    /* close termattr_path */
    cl_ret = close(f);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));
    }
    return;

  }

  /*
   * close saved_termattr
   */
  cl_ret = close(f);
  if (cl_ret < 0) {

    /* report failed to save terminal state */
    if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
    }
    fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n", termattr_path, strerror (errno));

  }
  return;
}

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
  static char name[MU_BUF + 1] = {'\0'}; /* +1 for paranoia */
  struct passwd *pw;

  /*
   * do not modify name if it was already set
   */
  if (name[0] != '\0') {
      return name;
  }

  /*
   * lookup the password entry relating to the real user ID of the calling process
   */
  pw = getpwuid(getuid());

  /*
   * pre-load player name with rogo-
   */
  memset(name, 0, sizeof(name)); /* paranoia */
  strlcpy(name, "rogo-", sizeof(name));

  /*
   * paranoia check
   *
   * Only use the username of the real user ID of the calling process,
   * if the username is a non-empty string
   */
  if (pw != NULL && pw->pw_name != NULL && pw->pw_name[0] != '\0') {
      strlcat(name, pw->pw_name, sizeof(name));
  } else {
      strlcat(name, "nobody", sizeof(name));
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
 * ncurses_delete - free up ncurses state space
 */
static void
ncurses_delete(void)
{
    delwin (stdscr);
    delwin (curscr);
}

/*
 * endwin_and_ncurses_cleanup - flush output, shutdown ncurses if setup, and restore both echo + canonical mode
 */
void
endwin_and_ncurses_cleanup (void)
{
  /*
   * flush all output
   */
  fflush (stdout);
  fflush (stderr);

  /*
   * ncurses cleanup unless endwin() was already called
   */
  if (stdscr != NULL && !isendwin ()) {
  /*
   * move to corner of window
   */
    mvcur (0, C-1, R-1, 0);

  /*
   * turn on echo and turn off raw
   */
    (void) echo ();
    (void) noraw ();

  /*
   * clean up and delete curses
   */
    (void) endwin ();
    ncurses_delete ();
  }

  /*
   * restore previously saved terminal attributes
   */
  restore_termattr ();

  /*
    * output newline only once, even if this function is called several times
    *
    * NOTE: This function might be called via the atexit(3) facility, or as
    *	     a result of a signal handler, or both.  As a result we have
    *	     to guard against multiple calls to this function.
    */
  if (!final_newline) {
    putchar ('\n');
  }
  final_newline = true;
  fflush (stdout);
}

void
inter_endwin_and_ncurses_cleanup (int sig __attribute__ ((__unused__)))
{
  endwin_and_ncurses_cleanup();
  signal(sig, SIG_DFL);
  raise(sig);
}

/*
 * critical: Disable interrupts
 */

void
critical (void)
{
  hstat = signal (SIGHUP, SIG_IGN);
  istat = signal (SIGINT, SIG_IGN);
  pstat = signal (SIGPIPE, SIG_IGN);
  qstat = signal (SIGQUIT, SIG_IGN);
  tstat = signal (SIGTERM, SIG_IGN);
}

/*
 * uncritical: Enable interrupts
 */

void
uncritical (void)
{
  if (hstat != NULL) {
    signal (SIGHUP, hstat);
  } else {
    signal (SIGHUP, SIG_DFL);
  }
  if (istat != NULL) {
    signal (SIGINT, istat);
  } else {
    signal (SIGINT, SIG_DFL);
  }
  if (pstat != NULL) {
    signal (SIGPIPE, pstat);
  } else {
    signal (SIGPIPE, SIG_DFL);
  }
  if (qstat != NULL) {
    signal (SIGQUIT, qstat);
  } else {
    signal (SIGQUIT, SIG_DFL);
  }
  if (tstat != NULL) {
    signal (SIGTERM, tstat);
  } else {
    signal (SIGTERM, SIG_DFL);
  }
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
  signal (SIGTERM, SIG_DFL);
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

  if (signal (SIGTERM, SIG_IGN) != SIG_IGN) signal (SIGTERM, exitproc);
}

/*
 * form_path: calloc a path of a file under a directory
 *
 * If dir is NULL, then file is taken to be the full path.
 *
 * The caller must free the calloc-ed return value.
 *
 * This function will never return NULL.
 *
 * This function does NOT return on error.
 */
char *
form_path (const char *dir, const char *file)
{
  char *path;	/* path to open */
  int len;	/* length of path */

  /* firewall */
  /* dir can be NULL */
  if (file == NULL) {
    quit (1, "ERROR: %s: file is NULL\n", __func__);
    not_reached ();
  }

  /* form path */
  if (dir != NULL) {

    /* form full path */
    len = strlen (dir) + 1 + strlen (file);
    path = calloc (len + 1 + 1, 1); /* +1 for NUL, +1 for paranoia */
    if (path == NULL) {
      quit (1, "ERROR: %s: failed to calloc full path for %s/%s\n", __func__, dir, file);
      not_reached ();
    }
    snprintf(path, len+1, "%s/%s", dir, file); /* +1 for NUL */

  /* form file as a path */
  } else {

    /* form path */
    len = strlen (file);
    path = calloc (len + 1 + 1, 1); /* +1 for NUL, +1 for paranoia */
    if (path == NULL) {
      quit (1, "ERROR: %s: failed to calloc path for %s\n", __func__, file);
      not_reached ();
    }
    strlcpy(path, file, len+1);
  }

  return path;
}

/*
 * form_prefix_path: calloc a path of a file under a directory
 *
 * If dir is NULL, then file is taken to be the full path.
 *
 * The caller must free the calloc-ed return value.
 *
 * This function will never return NULL.
 *
 * This function does NOT return on error.
 */
char *
form_prefix_path (const char *dir, const char *prefix, const char *file)
{
  char *path;	/* path to open */
  int len;	/* length of path */

  /* firewall */
  /* dir can be NULL */
  if (prefix == NULL) {
    quit (1, "ERROR: %s: prefix is NULL\n", __func__);
    not_reached ();
  }
  if (file == NULL) {
    quit (1, "ERROR: %s: file is NULL\n", __func__);
    not_reached ();
  }

  /* form path */
  if (dir != NULL) {

    /* form full path */
    len = strlen (dir) + 1 + strlen(prefix) + strlen (file);
    path = calloc (len + 1 + 1, 1); /* +1 for NUL, +1 for paranoia */
    if (path == NULL) {
      quit (1, "ERROR: %s: failed to calloc full path for %s/%s\n", __func__, dir, file);
      not_reached ();
    }
    snprintf(path, len+1, "%s/%s%s", dir, prefix, file); /* +1 for NUL */

  /* form file as a path */
  } else {

    /* form path */
    len = strlen(prefix) + strlen (file);
    path = calloc (len + 1 + 1, 1); /* +1 for NUL, +1 for paranoia */
    if (path == NULL) {
      quit (1, "ERROR: %s: failed to calloc path for %s\n", __func__, file);
      not_reached ();
    }
    snprintf(path, len+1, "%s%s", prefix, file); /* +1 for NUL */
  }

  return path;
}

/*
 * lock_file: lock a file for a maximum number of seconds.
 *            Based on the method used in Rogue 5.2.
 */

int
lock_file (const char *caller, const char *dir, const char *lokfil)
{
  char *path;	     /* path to open */
  int ret;	     /* flock return */
  int lock_fd;	     /* opened locked file descriptor */

  /* firewall */
  if (caller == NULL) {
    quit (1, "ERROR: %s: caller is NULL\n", __func__);
    not_reached ();
  }
  /* is it OK for dir == NULL */
  if (lokfil == NULL) {
    quit (1, "ERROR: caller: %s: lokfil is NULL\n", caller);
    not_reached ();
  }

  /*
   * form lock path if needed
   */
  path = form_path (dir, lokfil);

  /*
   * open lock file
   */
  lock_fd = open (path, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); /* mode 0644 */
  if (lock_fd < 0) {
    /* failed to open and/or create the lock file */
    quit (1, "ERROR: %s: failed to open lock file: %s: %s\n", caller, path, strerror (errno));
    not_reached ();
  }

  /*
   * lock the file
   */
  ret = flock (lock_fd, LOCK_EX);
  if (ret < 0) {
    /* failed to lock */
    quit (1, "ERROR: %s: failed to lock: %s: %s\n", caller, path, strerror (errno));
    not_reached ();
  }

  /* free memory */
  if (path != NULL) {
    free(path);
    path = NULL;
  }

  /*
   * return the successful lock file descriptor
   */
  return lock_fd;
}

/*
 * unlock_file: Unlock a lock file.
 */

void
unlock_file (const char *caller, int lock_fd)
{
  int ret;	    /* flock return */

  /* firewall */
  if (caller == NULL) {
    quit (1, "ERROR: %s: caller is NULL\n", __func__);
    not_reached ();
  }

  /*
   * do nothing is lock file is not open
   */
  if (lock_fd < 0) {
    /* not open for unlocking */
    return;
  }

  /*
   * unlock
   */
  ret = flock (lock_fd, LOCK_UN);
  if (ret < 0) {
    /* failed to lock */
    quit (1 , "ERROR: %s: failed to unlock\n", caller);
    not_reached ();
  }

  /*
   * close the lock
   */
  (void) close (lock_fd);

  /*
   * unlock successful
   */
  return;
}

# ifndef CMU
/*
 * quit: Defined for compatibility with Berkeley 4.2 system
 */

void
quit (int code, char *fmt, ...)
{
  va_list ap;

  /* pre-output newline to be on the edge of the screen before printing error message */
  fputc('\n', stderr);

  /* setup stdarg */
  va_start (ap, fmt);

  /* print error message to stderr */
  vfprintf (stderr, fmt, ap);
  fflush(stderr); /* paranoia */

  /* finish stdarg */
  va_end (ap);

  /* exit :-) */
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
