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

# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <sys/types.h>
# include <string.h>
# include <ctype.h>
# include <sys/stat.h>
# include <time.h>
# include <fcntl.h>
# include <errno.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "types.h"
# include "config.h"

/*
 * global declarations
 */
char rgmdir[MU_BUF + 1] = { '\0' };	/* rogomatic directory - may include UTC date and time sub-dir, +1 for paranoia */
char lock_path[TY_BUF + 1] = { '\0' };  /* rogomatic lock file path, +1 for paranoia */

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
  int rgmdirlen;		/* length of the rogomatic directory path plus trailing slash "/" */
  int lockpathlen;		/* length of the rogomatic lock file */
  int datelen;			/* length of the UTC date string */
  struct stat rgmdir_stat;	/* stat of rgmdir */
  int ret;			/* stat(2) return */

  /*
   * if needed, load the RGMDIR default
   *
   * NOTE: It is possible for some non-default path to be loaded in already
   */
  if (rgmdir[0] == '\0') {
    memset (rgmdir, 0, sizeof(rgmdir));
    strlcpy (rgmdir, RGMDIR, sizeof(rgmdir));
  }
  rgmdirlen = strlen (rgmdir);

  /*
   * create the main rogomatic directory if it does not already exist
   */
  memset (&rgmdir_stat, 0, sizeof(rgmdir_stat));
  ret = stat(rgmdir, &rgmdir_stat);
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
  ret = stat(rgmdir, &rgmdir_stat);
  if (ret < 0 || ((rgmdir_stat.st_mode & S_IFDIR) == 0)) {
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
  if (time_subpath && !subpath_formed) {

    /*
     * determine now in UTC
     */
    tm = time (NULL);
    if (tm == -1) {
      fprintf (stderr, "Warning: %s: using default RGMDIR: %s: failed to get current time\n", __func__, rgmdir);
      return;
    }
    utc_now = gmtime (&tm);
    if (utc_now == NULL) {
      fprintf (stderr, "Warning: %s: using default RGMDIR: %s: failed convert now into UTC now\n", __func__, rgmdir);
      return;
    }

    /*
     * attempt to append a UTC date and time sub-directory
     */
    if (rgmdirlen >= sizeof(rgmdir)-1) {
      fprintf (stderr, "Warning: %s: using default RGMDIR: %s: RGMDIR already too long to append UTC date & time sub-dir\n",
		       __func__, rgmdir);
      return;
    }
    rgmdir[rgmdirlen] = '/';
    ++rgmdirlen;
    datelen = sizeof ("YYYYMMDD_HHMMSS");
    if (rgmdirlen+datelen >= sizeof(rgmdir)-1) {
      fprintf (stderr, "Warning: %s: using default RGMDIR: %s: RGMDIR too long to append UTC date & time sub-dir\n",
		       __func__, rgmdir);
      return;
    }
    if (strftime (rgmdir + rgmdirlen, datelen, "%Y%m%d_%H%M%S", utc_now) == 0) {
      fprintf (stderr, "Warning: %s: using default RGMDIR: %s: failed to convert time to string\n", __func__, rgmdir);
      return;
    }
    rgmdirlen = strlen (rgmdir);

    /*
     * create the rogomatic sub-directory if it does not already exist
     */
    memset (&rgmdir_stat, 0, sizeof(rgmdir_stat));
    ret = stat(rgmdir, &rgmdir_stat);
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
    ret = stat(rgmdir, &rgmdir_stat);
    if (ret < 0 || ((rgmdir_stat.st_mode & S_IFDIR) == 0)) {
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
    lockpathlen = rgmdirlen + strlen ("/Rgm.Lock");
    if (lockpathlen >= sizeof(lock_path)-1) {
      fprintf (stderr, "ERROR: %s: RGMDIR %s: too long to form lock file path\n", __func__, rgmdir);
      exit (1);
    }
    snprintf (lock_path, lockpathlen + 1, "%s/Rgm.Lock", rgmdir); /* +1 for paranoia */
  }
  return;
}
