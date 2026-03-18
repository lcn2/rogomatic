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
# include <sys/stat.h>
# include <time.h>

# include "types.h"
# include "install.h"

/*
 * globals
 */
char rgmdir[SM_BUF + 1] = { '\0' };	/* rogomatic directory, +1 for paranoia */
char lock_path[SM_BUF + 1] = { '\0' };	/* rogomatic lock file path, +1 for paranoia */
int time_subpath = 0;			/* 0 ==> do not append UTC date/time to rgmdir, != 0 ==> append */

/*
 * set_rgmdir: setup the rogomatic directory path band the path of the lock file
 *
 * Given:
 *
 *	time_subpath == 0 ==> use the RGMDIR default
 *	else ==> append a date and time subpath
 */
void
set_rgmdir (void)
{
  time_t tm;			/* now */
  struct tm *utc_now;		/* tm in UTC */
  int rgmdirlen;		/* length of the default rogomatic directory path plus trailing slash "/" */
  int datelen;			/* length of the UTC date string */

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
   * if needed, form the rogomatic lock file path
   *
   * NOTE: The state of time_subpath does NOT impact the crogomatic lock file path.
   */
  if (lock_path[0] == '\0') {
    memset (lock_path, 0, sizeof(lock_path));
    snprintf (lock_path, SM_BUF, "%s/Rgm.Lock", rgmdir);
  }

  /*
   * if we are to add a time_subpath, attempt to append the UTC date and time
   */
  if (time_subpath) {

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
  }
  return;
}
