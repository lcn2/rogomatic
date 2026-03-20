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
# include <unistd.h>
# include <string.h>
# include <ctype.h>
# include <sys/stat.h>
# include <time.h>
# include <fcntl.h>
# include <errno.h>

# include "types.h"
# include "globals.h"
# include "install.h"

/*
 * static declarations
 */
static bool subpath_formed = false;	/* true ==> we haven't appended UTC date/time to rgmdir, false ==> we have */

/*
 * set_rgmdir: setup the rogomatic directory path and the rogomatic lock file path
 *
 * Given:
 *
 *	time_subpath == 0 ==> use the RGMDIR default
 *	else ==> append a date and time subpath
 */
void
set_rgmdir (bool time_subpath)
{
  time_t tm;			/* now */
  struct tm *utc_now;		/* tm in UTC */
  int rgmdirlen;		/* length of the default rogomatic directory path plus trailing slash "/" */
  int datelen;			/* length of the UTC date string */
  struct stat rgmdir_buf;	/* stat of rgmdir */
  int ret;			/* stat(2) return */

  /*
   * if needed, load the RGMDIR default
   *
   * NOTE: It is possible for some non-default path to be loaded in already
   */
  if (rgmdir[0] == '\0') {
    memset (rgmdir, 0, sizeof(rgmdir));
    strncpy (rgmdir, RGMDIR, SM_BUF);
  }

  /*
   * create the main rogomatic directory if it does not already exist
   */
  memset (&rgmdir_buf, 0, sizeof(rgmdir_buf));
  ret = stat(rgmdir, &rgmdir_buf);
  if (ret < 0) {
      /* no rgmdir, attempt to mkdir(rgmdir) */
      ret = mkdir(rgmdir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH); /* mkdir -m 0755 rgmdir */
      if (ret < 0) {
	fprintf (stderr, "ERROR: %s: mkdir %s failed: %s\n", __func__, rgmdir, strerror (errno));
	exit (1);
      }
  }

  /*
   * verify that main rogomatic directory is a read-write searchable directory
   */
  ret = stat(rgmdir, &rgmdir_buf);
  if (ret < 0 || ((rgmdir_buf.st_mode & S_IFDIR) == 0)) {
    fprintf (stderr, "ERROR: %s: not a directory: %s: %s\n", __func__, rgmdir, strerror (errno));
    exit (1);
  }
  ret = access(rgmdir, R_OK|W_OK|X_OK);
  if (ret < 0) {
    fprintf (stderr, "ERROR: %s: directory is not read-write and searchable: %s\n", __func__, rgmdir);
    exit (1);
  }


  /*
   * if we are to add a time_subpath, attempt to append the UTC date and time
   */
  if (!time_subpath && !subpath_formed) {

    /*
     * determine now in UTC
     */
    tm = time (NULL);
    if (tm == -1) {
      fprintf (stderr, "ERROR: %s: failed to get current time\n", __func__);
      return;
    }
    utc_now = gmtime (&tm);
    if (utc_now == NULL) {
      fprintf (stderr, "ERROR: %s: failed convert now into UTC now\n", __func__);
      return;
    }

    /*
     * attempt to append a UTC date and time sub-directory
     */
    rgmdirlen = strlen (rgmdir); /* +1 for trailing slash "/" */
    if (rgmdirlen >= SM_BUF) {
      fprintf (stderr, "ERROR: %s: RGMDIR default is already too long to append UTC date and time sub-dir\n",
		       __func__);
      return;
    }
    rgmdir[rgmdirlen] = '/';
    ++rgmdirlen;
    datelen = sizeof ("YYYYMMDD_HHMMSS");
    if (rgmdirlen+datelen >= SM_BUF) {
      fprintf (stderr, "ERROR: %s: RGMDIR default is too long to append UTC date and time sub-dir\n",
		       __func__);
      return;
    }
    if (strftime (rgmdir + rgmdirlen, datelen, "%Y%m%d_%H%M%S", utc_now) == 0) {
      fprintf (stderr, "ERROR: %s: failed to convert time to string\n", __func__);
    }

    /*
     * create the rogomatic sub-directory if it does not already exist
     */
    memset (&rgmdir_buf, 0, sizeof(rgmdir_buf));
    ret = stat(rgmdir, &rgmdir_buf);
    if (ret < 0) {
	/* no rgmdir, attempt to mkdir(rgmdir) */
	ret = mkdir(rgmdir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH); /* mkdir -m 0755 rgmdir */
	if (ret < 0) {
	  fprintf (stderr, "ERROR: %s: sub-dir mkdir %s failed: %s\n", __func__, rgmdir, strerror (errno));
	  exit (1);
	}
    }

    /*
     * verify that rogomatic sub-directory is a read-write searchable directory
     */
    ret = stat(rgmdir, &rgmdir_buf);
    if (ret < 0 || ((rgmdir_buf.st_mode & S_IFDIR) == 0)) {
      fprintf (stderr, "ERROR: %s: not a sub-directory: %s: %s\n", __func__, rgmdir, strerror (errno));
      exit (1);
    }
    ret = access(rgmdir, R_OK|W_OK|X_OK);
    if (ret < 0) {
      fprintf (stderr, "ERROR: %s: sub-directory is not read-write and searchable: %s\n", __func__, rgmdir);
      exit (1);
    }

    /* note that the sub-directory has been formed so we don't repeat this action */
    subpath_formed = true;
  }

  /*
   * if needed, form the rogomatic lock file path
   *
   * NOTE: The state of time_subpath does NOT impact the crogomatic lock file path.
   */
  if (lock_path[0] == '\0') {
    memset (lock_path, 0, sizeof(lock_path));
    snprintf (lock_path, SM_BUF, "%s/Rgm.Lock", rgmdir);
  }
  return;
}
