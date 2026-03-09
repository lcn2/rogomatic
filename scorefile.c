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
 * scorefile.c:
 *
 * This file contains the functions which update the rogomatic scorefile,
 * which lives in <RGMDIR>/rgmscore<versionstr>. LOCKFILE is used to
 * prevent simultaneous accesses to the file. rgmdelta<versionstr>
 * contains new scores, and whenever the score file is printed the delta
 * file is sorted and merged into the rgmscore file.
 */

# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <signal.h>
# include <string.h>
# include <errno.h>

# include "types.h"
# include "globals.h"
# include "install.h"

# define LINESIZE	2048
# define SCORE(s,p)     (atoi (s+p))

/* static declarations */

static char lokfil[MU_BUF + 1];

static void intrupscore (int sig __attribute__ ((__unused__)));

/*
 * add_score: Write a new score line out to the correct rogomatic score
 * file by creating a temporary copy and inserting the new line in the
 * proper place. Be tense about preventing simultaneous access to the
 * score file and catching interrupts and things.
 */

void
add_score (char *new_line, char *vers, int ntrm)
{
  int   wantscore = 1;
  char  ch;
  char  newfil[MU_BUF + 1]; /* new filename, +1 for paranoia */
  FILE *newlog;
  int lock_fd;

  /* zeroize arrays */
  memset (lokfil, 0, sizeof(lokfil)); /* paranoia */
  memset (newfil, 0, sizeof(newfil)); /* paranoia */

  snprintf (lokfil, MU_BUF, "%s.%s", LOCKFILE, vers);
  snprintf (newfil, MU_BUF, "%s/rgmdelta%s", getRgmDir (), vers);

  /* Defer interrupts while mucking with the score file */
  critical ();

  /* lock */
  lock_fd = lock_file (__func__, NULL, lokfil);

  /* Now create a temporary to copy into */
  if ((newlog = wopen (newfil, "a")) == NULL) {
    quit (1, "ERROR: %s: Unable to write: %s: %s\n", __func__, newfil, strerror (errno));
    not_reached ();
  } else {
    fprintf (newlog, "%s\n", new_line);
    fflush (newlog);
    fclose (newlog);
  }

  /* unlock */
  unlock_file (__func__, lock_fd);

  uncritical ();
}

/*
 * dumpscore: Print out the scoreboard.
 */

void
dumpscore (char *vers)
{
  char  ch;
  char  scrfil[MU_BUF + 1]; /* rogomatic score file path, +1 for paranoia */
  char  delfil[MU_BUF + 1]; /* rogomatic delta file path, +1 for paranoia */
  char  newfil[MU_BUF + 1]; /* rogomatic new file path, +1 for paranoia */
  char  allfil[MU_BUF + 1]; /* rogomatic all scores file path, +1 for paranoia */
  char  cmd[BIGBUF + 1]; /* shell command buffer, +1 for paranoia */
  FILE *scoref, *deltaf;
  int   oldmask;
  int   lock_fd;

  /* zeroize arrays */
  memset (lokfil, 0, sizeof(lokfil)); /* paranoia */
  memset (scrfil, 0, sizeof(scrfil)); /* paranoia */
  memset (delfil, 0, sizeof(delfil)); /* paranoia */
  memset (newfil, 0, sizeof(newfil)); /* paranoia */
  memset (allfil, 0, sizeof(allfil)); /* paranoia */
  memset (cmd, 0, sizeof(cmd)); /* paranoia */

  snprintf (lokfil, MU_BUF, "%s.%s", LOCKFILE, vers);
  snprintf (scrfil, MU_BUF, "%s/rgmscore%s", getRgmDir (), vers);
  snprintf (delfil, MU_BUF, "%s/rgmdelta%s", getRgmDir (), vers);
  snprintf (newfil, MU_BUF, "%s/NewScore%s", getRgmDir (), vers);
  snprintf (allfil, MU_BUF, "%s/AllScore%s", getRgmDir (), vers);

  /* On interrupts we must relinquish control of the score file */
  int_exit (intrupscore);

  /* lock */
  lock_fd = lock_file (__func__, NULL, lokfil);

  deltaf = fopen (delfil, "r");
  scoref = fopen (scrfil, "r");

  /* If there are new scores, sort and merge them into the score file */
  if (deltaf != NULL) {
    fclose (deltaf);

    /* Defer interrupts while mucking with the score file */
    critical ();

    /* Make certain any new files are world writeable */
    oldmask = umask (0);

    /* If we have an old file and a delta file, merge them */
    if (scoref != NULL) {
      fclose (scoref);
      snprintf (cmd, BIGBUF, "sort +4nr -o %s %s; sort -m +4nr -o %s %s %s",
               newfil, delfil, allfil, newfil, scrfil);
      system (cmd);

      if (filelength (allfil) != filelength (delfil) + filelength (scrfil)) {
	/* unlink */
	unlink (newfil);
	unlink (allfil);

	/* unlock */
	unlock_file (__func__, lock_fd);
	quit (1, "ERROR: %s: new file is wrong length!\n", __func__);
	not_reached ();
      }
      else {
        /* New file is okay, unlink old files and pointer swap score file */
        unlink (delfil); unlink (newfil);
        unlink (scrfil); link (allfil, scrfil); unlink (allfil);
      }

      scoref = fopen (scrfil, "r");
    }
    else
      /* Only have delta file, sort into scorefile and unlink delta */
    {
      snprintf (cmd, BIGBUF, "sort +4nr -o %s %s", scrfil, delfil);
      system (cmd);
      unlink (delfil);
      scoref = fopen (scrfil, "r");
    }

    /* Restore umask */
    umask (oldmask);

    /* Restore interrupt status after score file stable */
    uncritical ();
  }

  /* Now any new scores have been put into scrfil, read it */
  if (scoref == NULL) {

    /* unlock */
    unlock_file (__func__, lock_fd);
    quit (1, "ERROR: %s: Can't find: %s\nBest score was: %d\n", __func__, scrfil, BEST);
    not_reached ();
  }

  printf ("Rog-O-Matic Scores against version %s:\n\n", vers);
  printf ("%s%s", "Date         User        Gold    Killed by",
          "      Lvl  Hp  Str  Ac  Exp\n\n");

  while ((int) (ch = fgetc (scoref)) != EOF)
    putchar (ch);

  fclose (scoref);

  /* unlock */
  unlock_file (__func__, lock_fd);

  exit (0);
}

/*
 * intrupscore: We have an interrupt, clean up and unlock the score file.
 */

static void
intrupscore (int sig __attribute__ ((__unused__)))
{
  exit (1);
}
