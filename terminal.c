/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2026  Landon Curt Noll
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

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/file.h>
# include <termios.h>
# include <errno.h>
# include <sys/stat.h>

# include "types.h"
# include "config.h"
# include "globals.h"

static char *termattr_path = NULL;	/* path of the saved_termattr file or NULL */
static int termattr_fd = -1;		/* open file descriptor of saved_termattr file or -1 */

/*
 * cleanup_termattr: cleanup any use of the saved_termattr file
 */

static void
cleanup_termattr (bool unlink_saved_termattr)
{
  ssize_t cl_ret;		 /* close() return status */
  int ret;			 /* unlink() return status */

  /*
   * close saved_termattr file if it was open
   */
  if (termattr_fd >= 0) {
    cl_ret = close(termattr_fd);
    if (cl_ret < 0) {
      fprintf(stderr, "Warning: close of: %s failed: %s\n",
		      (termattr_path == NULL ? "saved_termattr" : termattr_path), strerror (errno));
    }
  }

  /*
   * unlink saved_termattr file if it exists
   */
  if (unlink_saved_termattr && termattr_path != NULL) {
    ret = unlink (termattr_path);
    if (ret < 0) {
      fprintf(stderr, "Warning: unlink of %s failed: %s\n", termattr_path, strerror (errno));
    }
  }

  /*
   * free saved_termattr file path if needed
   */
  if (termattr_path != NULL) {
    free (termattr_path);
    termattr_path = NULL;
  }
  return;
}

/*
 * save_termattr: save stdin, stdout, and stderr terminal state into saved_termattr file
 *
 * NOTE: If dir is NULL or empty string, then rgmdir is used if non-empty string, else RGMDIR is used as dir.
 *
 * NOTE: If stdin, stdout, or stderr is NOT associated with a tty, then we simply save the zeroized struct termios.
 *
 * returns:
 *
 *  false ==> failed to save stdin, stdout, and stderr terminal state
 *  true ==> successfully saved stdin, stdout, and stderr terminal state
 */

bool
save_termattr (char *dir)
{
  struct termios tattr_stdin;    /* stdin terminal state */
  struct termios tattr_stdout;   /* stdout terminal state */
  struct termios tattr_stderr;   /* stderr terminal state */
  int ret;			 /* tcgetattr() return status */
  ssize_t wr_ret;		 /* write() return status */

  /*
   * special case: NULL dir, or dir as an empty string
   */
  if (dir == NULL || dir[0] == '\0') {
    if (rgmdir[0] == '\0') {
      dir = RGMDIR;
    } else {
      dir = rgmdir;
    }
  }
  /* assert: dir is a non-NULL, non-empty string */

  /*
   * firewall - close any existing open termattr_path file
   */
  if (termattr_fd >= 0) {
      (void) close (termattr_fd); /* force termattr_fd closed - do not care if this fails */
      termattr_fd = -1; /* indicate saved_termattr file is now closed */
  }

  /*
   * firewall - free any existing termattr_path file path
   */
  if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
  }
  /* assert: termattr_fd < 0, termattr_fd closed, termattr_path is NULL */

  /*
   * obtain the stdin, stdout, and stderr terminal states
   *
   * NOTE: If stdin, stdout, or stderr isn't a terminal, then we let the zeroized struct termios
   *       remain and write out that set of zeros as our "state".
   */
  memset (&tattr_stdin, 0, sizeof(tattr_stdin));
  if (isatty (STDIN_FILENO)) {
    ret = tcgetattr (STDIN_FILENO, &tattr_stdin);
    if (ret != 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: tcgetattr of stdin state failed: %s",
		      strerror (errno));
      return false;
    }
  }
  /**/
  memset (&tattr_stdout, 0, sizeof(tattr_stdout));
  if (isatty (STDOUT_FILENO)) {
    ret = tcgetattr (STDOUT_FILENO, &tattr_stdout);
    if (ret != 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: tcgetattr of stdout state failed: %s",
		       strerror (errno));
      return false;
    }
  }
  /**/
  memset (&tattr_stderr, 0, sizeof(tattr_stderr));
  if (isatty (STDERR_FILENO)) {
    ret = tcgetattr (STDERR_FILENO, &tattr_stderr);
    if (ret != 0) {
      fprintf(stderr, "Warning: terminal attributes not saved: tcgetattr of stderr state failed: %s",
		      strerror (errno));
      return false;
    }
  }

  /*
   * form saved_termattr path
   */
  termattr_path = form_path (dir, "saved_termattr");

  /*
   * open for writing (create if needed with more 0644) saved_termattr
   */
  termattr_fd = open (termattr_path, O_WRONLY|O_TRUNC|O_APPEND|O_CREAT|O_CLOEXEC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); /* mode 0644 */
  if (termattr_fd < 0) {

    /* indicate that saved_termattr is not setup */
    fprintf(stderr, "Warning: terminal attributes not saved: open: %s failed: %s\n",
		    termattr_path, strerror (errno));
    cleanup_termattr (true);
    return false;

  }

  /*
   * write stdin, stdout, and stderr terminal states to the saved_termattr file
   */
  wr_ret = write (termattr_fd, &tattr_stdin, sizeof(tattr_stdin));
  if (wr_ret < 0) {

    /* report failed to save terminal state */
    fprintf(stderr, "Warning: terminal attributes not saved: write of stdin state failed: %s",
		    strerror (errno));
    cleanup_termattr (true);
    return false;

  } else if (wr_ret != sizeof(tattr_stdin)) {

    /* report failed to save terminal state */
    fprintf(stderr, "Warning: terminal attributes not saved: write size of stdin state: %zd != %zu",
		    wr_ret, sizeof(tattr_stdin));
    cleanup_termattr (true);
    return false;

  }
  /**/
  wr_ret = write (termattr_fd, &tattr_stdout, sizeof(tattr_stdout));
  if (wr_ret < 0) {

    /* report failed to save terminal state */
    fprintf(stderr, "Warning: terminal attributes not saved: write of stdout state failed: %s",
	            strerror (errno));
    cleanup_termattr (true);
    return false;

  } else if (wr_ret != sizeof(tattr_stdout)) {

    /* report failed to save terminal state */
    fprintf(stderr, "Warning: terminal attributes not saved: write size of stdout state: %zd != %zu",
		    wr_ret, sizeof(tattr_stdout));
    cleanup_termattr (true);
    return false;

  }
  /**/
  wr_ret = write (termattr_fd, &tattr_stderr, sizeof(tattr_stderr));
  if (wr_ret < 0) {

    /* report failed to save terminal state */
    fprintf(stderr, "Warning: terminal attributes not saved: write of stderr state failed: %s",
		    strerror (errno));
    cleanup_termattr (true);
    return false;

  } else if (wr_ret != sizeof(tattr_stderr)) {

    /* report failed to save terminal state */
    fprintf(stderr, "Warning: terminal attributes not saved: write size of stderr state: %zd != %zu",
		    wr_ret, sizeof(tattr_stderr));
    cleanup_termattr (true);
    return false;
  }

  /*
   * terminal state for stdin, stdout, and stderr successfully saved
   */
  cleanup_termattr (false);
  return true;
}

/*
 * restore_termattr: restore terminal state for stdin, stdout, and stderr from the saved_termattr file
 *
 * NOTE: If dir is NULL or empty string, then rgmdir is used if non-empty string, else RGMDIR is used as dir.
 *
 * NOTE: If stdin, stdout, or stderr is NOT associated with a tty, then we do not restore a terminal state.
 *	 Also if the read struct termios is all zeros, then we assume that when save_termattr() was called the
 *	 associated descriptor wasn't associated with a terminal.  In that case we do not restore a terminal state.
 *
 * returns:
 *
 *  false ==> failed to restore stdin, stdout, and stderr terminal state
 *  true ==> successfully restored stdin, stdout, and stderr terminal state
 */

bool
restore_termattr (char *dir)
{
  struct termios tattr_stdin;   /* stdin terminal state */
  struct termios tattr_stdout;  /* stdout terminal state */
  struct termios tattr_stderr;  /* stderr terminal state */
  struct termios zero;		/* terminal state that has been zeroized */
  int ret;			/* tcsetattr() or fstat() return status */
  ssize_t rd_ret;		/* read() return status */
  ssize_t cl_ret;		/* close() return status */
  struct stat stat_buf;		/* termattr_path file status */

  /*
   * firewall - NULL dir becomes rgmdir
   */
  if (dir == NULL || dir[0] == '\0') {
    if (rgmdir[0] == '\0') {
      dir = RGMDIR;
    } else {
      dir = rgmdir;
    }
  }
  /* assert: dir is a non-NULL, non-empty string */

  /*
   * firewall - close any existing open termattr_path file
   */
  if (termattr_fd >= 0) {
      (void) close (termattr_fd); /* force termattr_fd closed - do not care if this fails */
      termattr_fd = -1; /* indicate saved_termattr file is now closed */
  }

  /*
   * firewall - free any existing termattr_path file path
   */
  if (termattr_path != NULL) {
      free (termattr_path);
      termattr_path = NULL;
  }
  /* assert: termattr_fd < 0, termattr_fd closed, termattr_path is NULL */

  /*
   * form saved_termattr path
   */
  termattr_path = form_path (dir, "saved_termattr");

  /*
   * open of reading saved_termattr
   */
  termattr_fd = open (termattr_path, O_RDONLY);
  if (termattr_fd < 0) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: open: %s failed: %s\n",
		    termattr_path, strerror (errno));
    cleanup_termattr (false);
    return false;

  }

  /*
   * check that the open termattr_path file has the correct size
   */
  ret = fstat (termattr_fd, &stat_buf);
  if (ret < 0) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: fstat on: %s failed: %s\n",
		    termattr_path, strerror (errno));
    cleanup_termattr (false);
    return false;
  }
  if (stat_buf.st_size != sizeof(tattr_stdin)+sizeof(tattr_stdout)+sizeof(tattr_stderr)) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: file: %s size: %lld != %zu\n",
		    termattr_path, (long long)stat_buf.st_size, sizeof(tattr_stdin)+sizeof(tattr_stdout)+sizeof(tattr_stderr));
    cleanup_termattr (false);
    return false;
  }

  /*
   * read stdin, stdout, and stderr terminal states from the saved_termattr file
   */
  memset (&tattr_stdin, 0, sizeof(tattr_stdin)); /* paranoia */
  rd_ret = read (termattr_fd, &tattr_stdin, sizeof(tattr_stdin));
  if (rd_ret < 0) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: read of stdin state failed: %s\n",
		    strerror (errno));
    cleanup_termattr (false);
    return false;

  } else if (rd_ret != sizeof(tattr_stdin)) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: read size of stdin state: %zd != %zu\n",
		    rd_ret, sizeof(tattr_stdin));
    cleanup_termattr (false);
    return false;

  }
  /**/
  memset (&tattr_stdout, 0, sizeof(tattr_stdout)); /* paranoia */
  rd_ret = read (termattr_fd, &tattr_stdout, sizeof(tattr_stdout));
  if (rd_ret < 0) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: read of stdout state failed: %s\n",
		    strerror (errno));
    cleanup_termattr (false);
    return false;

  } else if (rd_ret != sizeof(tattr_stdout)) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: read size of stdout state: %zd != %zu\n",
		    rd_ret, sizeof(tattr_stdout));
    cleanup_termattr (false);
    return false;

  }
  /**/
  memset (&tattr_stderr, 0, sizeof(tattr_stderr)); /* paranoia */
  rd_ret = read (termattr_fd, &tattr_stderr, sizeof(tattr_stderr));
  if (rd_ret < 0) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: read of stderr state failed: %s\n",
		    strerror (errno));
    cleanup_termattr (false);
    return false;

  } else if (rd_ret != sizeof(tattr_stderr)) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not restored: read size of stderr state: %zd != %zu\n",
		    rd_ret, sizeof(tattr_stderr));
    cleanup_termattr (false);
    return false;

  }

  /*
   * restore the stdin, stdout, and stderr terminal states
   */
  memset (&zero, 0, sizeof(zero));
  if (isatty (STDIN_FILENO)) {
    if (memcmp (&tattr_stdin, &zero, sizeof(tattr_stdin)) != 0) {
      ret = tcsetattr (STDIN_FILENO, TCSANOW, &tattr_stdin);
      if (ret != 0) {

	/* report failed to restore terminal state */
	fprintf(stderr, "Warning: terminal attributes not restored: tcsetattr of stdin state failed: %s\n",
			strerror (errno));
	cleanup_termattr (false);
	return false;

      }
    }
  }
  /**/
  if (isatty (STDOUT_FILENO)) {
    if (memcmp (&tattr_stdout, &zero, sizeof(tattr_stdout)) != 0) {
      ret = tcsetattr (STDOUT_FILENO, TCSANOW, &tattr_stdout);
      if (ret != 0) {

	/* report failed to restore terminal state */
	fprintf(stderr, "Warning: terminal attributes not restored: tcsetattr of stdout state failed: %s\n",
			strerror (errno));
	cleanup_termattr (false);
	return false;

      }
    }
  }
  /**/
  if (isatty (STDERR_FILENO)) {
    if (memcmp (&tattr_stderr, &zero, sizeof(tattr_stderr)) != 0) {
      ret = tcsetattr (STDERR_FILENO, TCSANOW, &tattr_stderr);
      if (ret != 0) {

	/* report failed to restore terminal state */
	fprintf(stderr, "Warning: terminal attributes not restored: tcsetattr of stderr state failed: %s\n",
			strerror (errno));
	cleanup_termattr (false);
	return false;

      }
    }
  }

  /*
   * close saved_termattr
   */
  cl_ret = close(termattr_fd);
  if (cl_ret < 0) {

    /* report failed to restore terminal state */
    fprintf(stderr, "Warning: terminal attributes not saved: close of: %s failed: %s\n",
		    termattr_path, strerror (errno));
    cleanup_termattr (false);
    return false;

  }
  termattr_fd = -1; /* indicate saved_termattr file is now closed */

  /*
   * terminal state for stdin, stdout, and stderr successfully restored
   */
  cleanup_termattr (true);
  return true;
}
