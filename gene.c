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
 * gene.c:
 *
 * Initialize and summarize the gene pool
 */

# include <stdio.h>
# include <stdlib.h>
# include <stdlib.h>
# include <string.h>

# include "types.h"
# include "install.h"

/*
 * external declarations
 */
extern void set_rgmdir (bool time_subpath);

/*
 * global declarations
 */
char rgmdir[MU_BUF + 1] = { '\0' };	/* rogomatic directory - may include UTC date and time sub-dir, +1 for paranoia */
char lock_path[TY_BUF + 1] = { '\0' };  /* rogomatic lock file path, +1 for paranoia */
char genelock[TY_BUF + 1] = { '\0' };	/* Gene pool lock file, +1 for paranoia */
char genepool[TY_BUF + 1] = { '\0' };	/* Gene pool, +1 for paranoia */
char *knob_name[MAXKNOB] = {
  "trap searching:   ",
  "door searching:   ",
  "resting:          ",
  "using arrows:     ",
  "experimenting:    ",
  "retreating:       ",
  "waking monsters:  ",
  "hoarding food:    "
};

int
main (int argc, char *argv[])
{
  char genelog[TY_BUF + 1];	/* Genetic learning log file, +1 for paranoia */
  int m=10, init=0, version=RV53A, full=0;
  unsigned int seed = 0;
  int lock_fd;

  /* zeroize arrays */
  memset (genelock, 0, sizeof(genelock)); /* paranoia */
  memset (genelog, 0, sizeof(genelog)); /* paranoia */
  memset (genepool, 0, sizeof(genepool)); /* paranoia */

  /* Get the options */
  while (--argc > 0 && (*++argv)[0] == '-') {
    while (*++(*argv)) {
      switch (**argv) {
        case 'a':	full=2; break;
        case 'i':	init++; break;
        case 'f':	full=1; break;
        case 'm':	m = atoi(*argv+1); SKIPARG;
          printf ("Gene pool size %d.\n", m);
          break;
        case 's':	seed = (unsigned int) atoi(*argv+1); SKIPARG;
          printf ("Random seed %d.\n", m);
          break;
        case 'v':	version = atoi(*argv+1); SKIPARG;
          printf ("Rogue version %d.\n", version);
          break;
        default:
	  quit (1, "Usage: gene [-if] [-m<<value>>] [-s<<value>>] [-v<<value>>] [genepool]\n");
	  not_reached ();
      }
    }
  }

  if (argc > 0) {
    if (readgenes (argv[0]))		/* Read the gene pool */
      analyzepool (full);		/* Print a summary */
    else {
      quit (0, "ERROR: %s: Cannot read file: %s\n", __func__, argv[0]);
      not_reached ();
    }

    exit (0);
  }

  /* No file argument, assign the gene log and pool file names */
  snprintf (genelock, sizeof(genelock)-1, "%s/GeneLock%d", rgmdir, version);
  snprintf (genelog, sizeof(genelog)-1, "%s/GeneLog%d", rgmdir, version);
  snprintf (genepool, sizeof(genepool)-1, "%s/GenePool%d", rgmdir, version);

  critical ();				/* Disable interrupts */

  lock_fd = lock_file (__func__, NULL, genelock);
  if (init) {
    rogo_srand (seed);			/* Set the random number generator */
    rogo_openlog (genelog);		/* Open the gene log file */
    initpool (MAXKNOB, m);		/* Random starting point */
    writegenes (genepool);		/* Write out the gene pool */
    rogo_closelog ();			/* Close the log file */
  }
  else if (! readgenes (genepool)) {	/* Read the gene pool */
    quit (1, "ERROR: %s: Cannot read file '%s'\n", __func__, genepool);
    not_reached ();
  }
  unlock_file (__func__, lock_fd);

  uncritical ();			/* Re-enable interrupts */
  analyzepool (full);			/* Print a summary */
}
