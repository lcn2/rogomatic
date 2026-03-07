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
 * findscore.c:
 *
 * Read the Rogue scoreboard to determine a goal score.
 */

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>

# include "install.h"
# include "types.h"
# include "globals.h"

# define TEMPFL "/tmp/RscoreXXXXXX"
# define ISDIGIT(c) ((c) >= '0' && (c) <= '9')

int
findscore (char *rogue, char *roguename)
{
  int score, best = -1;
  char cmd[TY_BUF + 1]; /* rogue -s command, +1 for paranoia */
  char buffer[BUFSIZ + 1]; /* read buffer, +1 for paranoia */
  char *s;
  char tmpfname[MU_BUF + 1];
  FILE *tmpfil;
  int fd;

  /* zeroize arrays */
  memset (cmd, 0, sizeof(cmd)); /* paranoia */
  memset (buffer, 0, sizeof(buffer)); /* paranoia */
  memset (tmpfname, 0, sizeof(tmpfname)); /* paranoia */

  fd = mkstemp(tmpfname);

  if (fd > 0) {
    /* Run 'rogue -s', and put the scores into a temp file */
    snprintf (cmd, MU_BUF, "%s -s >%s", rogue, TEMPFL);
    system (cmd);
  }

  /* If no temp file created, return default score */
  if (fd <= 0 || (tmpfil = fdopen(fd,"r")) == NULL)
    return (best);

  /* Skip to the line starting with 'Rank...'. */
  while (fgets (buffer, BUFSIZ, tmpfil) != NULL)
    if (stlmatch (buffer, "Rank")) break;

  if (! feof (tmpfil)) {
    while (fgets (buffer, BUFSIZ, tmpfil) != NULL) {
      s = buffer;				/* point s at buffer */

      while (ISDIGIT (*s)) s++;			/* Skip over rank */

      while (*s == ' ' || *s == '\t') s++;	/* Skip to score */

      score = atoi (s);				/* Read score */

      while (ISDIGIT (*s)) s++;			/* Skip over score */

      while (*s == ' ' || *s == '\t') s++;	/* Skip to player */

      if (stlmatch (s, roguename)) {	/* Found our heros name */
        if (best < 0) best = score;		/* Rogy is on top! */

        break;					/* 'best' is now target */
      }

      if (score < BOGUS &&
          (score < best || best < 0))		/* Save smallest score */
        best = score;				/*  above Rogy's score */
    }
  }

  unlink (tmpfname);

  /* Don't quit for very small scores, it's not worth it */
  if (best < 2000) best = -1;

  return (best);
}
