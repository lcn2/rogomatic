/*
 * fork_exec - fork a while process and execute a file
 *
 * Copyright (c) 2026 by Landon Curt Noll.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright, this permission notice and text
 * this comment, and the disclaimer below appear in all of the following:
 *
 *       supporting documentation
 *       source copies
 *       source works derived from this source
 *       binaries derived from this source or from derived source
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL LANDON CURT NOLL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * chongo (Landon Curt Noll) /\oo/\
 *
 * http://www.isthe.com/chongo/index.html
 * https://github.com/lcn2
 *
 * Share and enjoy!  :-)
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <inttypes.h>

#include "types.h"
#include "config.h"


/*
 * fork_exec: fork a while process and execute a command with args
 *
 * For a child process, and then execute a command (searched for along $PATH), when a list of arguments.
 *
 * given:
 *	file	    - path to file to exec
 *	arglist	    - NULL terminated list of args, starting with file (argv[] style)
 *
 * returns:
 *	exit code from command
 *
 * Example:
 *
 *	int ret;		(* child exit statis *)
 *	char *newfil;		(* new filename *)
 *	char *delfil;		(* delta filename *)
 *	char *arglist[] = {	(* command to execute and its args, NULL terminated *)
 *	    "sort", "+4nr", "-o", newfil, delfil, NULL
 *	};
 *
 *	ret = fork_exec("sort", arglist);
 *	if (ret != 0) {
 *	    quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed to: sort +4nr -o %s %s exit code: %d\n",
 *		     __func__, __FILE__, __LINE__, dnum, newfil, delfil, ret);
 *	    not_reached ();
 *	}
 */

int
fork_exec (char *file, char *arglist[])
{
  pid_t pid;		/* fork(2) return */
  pid_t wait_ret;	/* waitpid(2) return */
  int stat_loc;		/* status of the dead child process */
  int ret = 0;		/* execvp(3) or child exit return code */

  /*
   * fork a child process
   */
  pid = fork ();
  if (pid < 0) {
    quit (1, "ERROR: %s: file: %s line: %d dungeon: %u filed to fork: %s\n",
	            __func__, __FILE__, __LINE__, dnum, strerror (errno));
    not_reached ();
  }

  /*
   * child process - exec the command
   */
  if (pid == 0) {
    ret = execvp (file, arglist);
    if (ret < 0) {
      quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed to execvp: %s error: %s\n",
		      __func__, __FILE__, __LINE__, dnum, file, strerror (errno));
      not_reached ();
    }
    quit (1, "ERROR: %s: file: %s line: %d dungeon: %u child execvp returned with: %jd: error: %s\n",
                     __func__, __FILE__, __LINE__, dnum, (intmax_t) pid, strerror (errno));
    not_reached ();

  /*
   * parent process - monitor child process
   */
  } else {
    wait_ret = waitpid (pid, &stat_loc, 0);
    if (wait_ret < 0) {
      quit (1, "ERROR: %s: file: %s line: %d dungeon: %u failed to waitpid(%jd, &stat_loc, 0): %s\n",
		      __func__, __FILE__, __LINE__, dnum, (intmax_t) pid, strerror (errno));
      not_reached ();
    }
    ret = WEXITSTATUS(stat_loc);
  }

  /*
   * return child exit code
   */
  return ret;
}
