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

# include <stdio.h>
# include <sys/types.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>

# include "types.h"
# include "install.h"

/*
 * sane_path: test a path is POSIX portable safe
 *
 * POSIX portable safe path matches this regex:
 *
 *	^[/0-9A-Za-z][/0-9A-Za-z._+-]*$
 *
 * Return < 0 if the string is NOT POSIX portable safe.
 */
int
sane_path (char *path)
{
    char *p;

    /* firewall */
    if (path == NULL) {
	return -1;
    }

    /*
     * check each character for POSIX portable safe chars
     */
    /* first character can only be slash "/" or alphanumeric */
    if (path[0] != '/' &&  isalnum(path[0])) {
	return -1;
    }
    /* remaining characters can only be: [/0-9A-Za-z._+-] */
    for (p=path+1; *p != '\0'; ++p) {
	/* allowed characters are [/0-9A-Za-z._+-] */
	if (*p != '/' && !isalnum(*p) && *p != '.' && *p != '_' && *p != '+' && *p != '-') {
	    return -2;
	}

    }

    /*
     * string is POSIX portable safe
     */
    return 0;
}

/*
 * getRgmDir: Return the rogomatic directory path
 *
 * If RGMDIR environment variable was set, try to use it.
 * However, if RGMDIR environment variable to too long, or is not POSIX portable safe,
 * use the RGMDIR according to the definition in install.h.
 */

const char *
getRgmDir (void)
{
  char *path;	/* rogomatic directory path to return */

  /*
   * start with for RGMDIR environment variable value
   */
  path = getenv ("RGMDIR");
  if (path == NULL) {
    /* no such environment variable, use RGMDIR default */
    return RGMDIR;
  }

  /*
   * sanity check the RGMDIR environment variable value
   */
  if (strlen (path) >= TY_BUF) {
    /* RGMDIR environment variable too long, use RGMDIR default */
    return RGMDIR;
  }
  if (sane_path (path) < 0) {
    /* RGMDIR environment variable contains unsafe characters, use RGMDIR default */
    return RGMDIR;
  }

  /* path is sane */
  return path;
}

/*
 * getLockFile: Return the rogomatic lock file path
 *
 * If RGMDIR environment variable was set, try to use it.
 * However, if RGMDIR environment variable to too long, or is not POSIX portable safe,
 * use the LOCKFILE according to the definition in install.h.
 */

const char *
getLockFile (void)
{
  char *path;			/* rogomatic directory path */
  static char buf[SM_BUF + 1];	/* rogomatic lock file path to return, +1 for paranoia */

  /*
   * start with for RGMDIR environment variable value
   */
  path = getenv ("RGMDIR");
  if (path == NULL) {
    return LOCKFILE;
  }

  /*
   * sanity check the RGMDIR environment variable value
   */
  if (strlen (path) >= TY_BUF) {
    /* RGMDIR environment variable too long, use LOCKFILE default */
    return LOCKFILE;
  }
  if (sane_path (path) < 0) {
    /* RGMDIR environment variable contains unsafe characters, use LOCKFILE default */
    return LOCKFILE;
  }

  /*
   * form the rogomatic lock file path
   */
  memset (buf, 0, sizeof(buf));
  snprintf (buf, SM_BUF, "%s/Rgm.Lock", path);

  /* return the rogomatic lock file path */
  return buf;
}
