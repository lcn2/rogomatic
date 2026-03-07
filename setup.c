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
 * setup.c:
 *
 * This is the program which forks and execs the Rogue & the Player
 */

# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# include <string.h>
# include <sys/errno.h>

# include "types.h"
# include "install.h"

# define READ    0
# define WRITE   1

/* static declarations */

static int   frogue, trogue;

static void replaylog (char *pname, char *fname, char *options);

int
main (int argc, char *argv[])
{
  int   ptc[2], ctp[2];
  int   child, score = 0, oldgame = 0;
  int   cheat = 0, noterm = 1, echo = 0, nohalf = 0, replay = 0;
  int   emacs = 0, rf = 0, terse = 0, user = 0, quitat = 2147483647;
  char  *rfile = "", *rfilearg = "";
  char	options[MU_BUF + 1]; /* rogomatic options, +1 for paranoia */
  char  ropts[SM_BUF + 1]; /* rogue options, +1 for paranoia */
  char  roguename[MU_BUF + 1]; /* rogue player name, +1 for paranoia */
  char  *pfile = "";

  /* zeroize arrays */
  memset (options, 0, sizeof(options)); /* paranoia */
  memset (ropts, 0, sizeof(ropts)); /* paranoia */
  memset (roguename, 0, sizeof(roguename)); /* paranoia */

  while (--argc > 0 && (*++argv)[0] == '-') {
    while (*++(*argv)) {
      switch (**argv) {
        case 'c': cheat++;        break; /* Will use trap arrows! */
        case 'e': echo++;         break; /* Echo file to roguelog */
        case 'f': rf++;           break; /* Next arg is the rogue file */
        case 'h': nohalf++;       break; /* No halftime show */
        case 'p': replay++;       break; /* Play back roguelog */
        case 'r': oldgame++;      break; /* Use saved game */
        case 's': score++;        break; /* Give scores only */
        case 't': terse++;        break; /* Give status lines only */
        case 'u': user++;         break; /* Start up in user mode */
        case 'w': noterm = 0;     break; /* Watched mode */
        case 'E': emacs++;        break; /* Emacs mode */
        default:  printf
          ("Usage: rogomatic [-cefhprstuwE] or rogomatic [file]\n");
          exit (1);
      }
    }

    if (rf) {
      if (--argc) rfilearg = *++argv;

      rf = 0;
    }
  }

  if (argc > 1) {
    printf ("Usage: rogomatic [-cefhprstuwE] or rogomatic <file>\n");
    exit (1);
  }

  /*
   * Find which rogue executable to use
   */
  if (*rfilearg) {
    if (access (rfilearg, R_OK|X_OK) == 0) {
	rfile = rfilearg;
    }
    else {
	fprintf (stderr, "ERROR: rogue arg not executable: %s - %s\n", rfilearg, strerror(errno));
	exit (1);
    }
  }
  else if (access ("./rogue", R_OK|X_OK) == 0) {
      rfile = "./rogue";
  }
# ifdef NEWROGUE
  else if (access (NEWROGUE, R_OK|X_OK) == 0) {
      rfile = NEWROGUE;
  }
# endif
# ifdef ROGUE
  else if (access (ROGUE, R_OK|X_OK) == 0) {
      rfile = ROGUE;
  }
# endif
  else {
    fprintf (stderr, "ERROR: rogue not found\n");
    exit (1);
  }

  /*
   * Find which player executable to use
   */
  if (access ("./player", R_OK|X_OK) == 0) {
      pfile = "./player";
  }
# ifdef PLAYER
  else if (access (PLAYER, R_OK|X_OK) == 0) {
      pfile = PLAYER;
  }
# endif
  else {
    fprintf (stderr, "ERROR: player not found\n");
    exit (1);
  }

  if (!replay && !score) quitat = findscore (rfile, "Rog-O-Matic");

  snprintf (options, MU_BUF, "%d,%d,%d,%d,%d,%d,%d,%d",
           cheat, noterm, echo, nohalf, emacs, terse, user, quitat);
  snprintf (roguename, MU_BUF, "Rog-O-Matic %s for %s", RGMVER, getname ());
  snprintf (ropts, SM_BUF, "name=%s,fruit=%s,%s,%s,%s,%s,%s,%s,inven=%s,%s,%s,file=%s/%s,score=%s/%s,lock=%s/%s",
            getname (), "apricot", "terse", "noflush", "noask",
            "jump", "step", "nopassgo", "slow", "seefloor", "tombstone",
	    RGMDIR, "rogue.sav", RGMDIR, "rogue.scr", RGMDIR, "rogue.lck");

  if (score)  { dumpscore (argc==1 ? argv[0] : DEFVER); exit (0); }

  if (replay) { replaylog (pfile, argc==1 ? argv[0] : ROGUELOG, options); exit (0); }

  if ((pipe (ptc) < 0) || (pipe (ctp) < 0)) {
    fprintf (stderr, "Cannot get pipes!\n");
    exit (1);
  }

  trogue = ptc[WRITE];
  frogue = ctp[READ];

  if ((child = fork ()) == 0) {
    close (0);
    dup (ptc[READ]);
    close (1);
    dup (ctp[WRITE]);

    if (setenv ("TERM", "vt100", 1) != 0) {
      fprintf (stderr, "ERROR: can't setenv (\"TERM\", \"vt100\", 1)\n");
      exit (1);
    }

    if (setenv ("ROGUEOPTS", ropts, 1) != 0) {
      fprintf (stderr, "ERROR: can't setenv (\"ROGUEOPTS\", \"%s\", 1)\n",ropts);
      exit (1);
    }

    if (oldgame)  execl (rfile, rfile, "-r", NULL);

    if (argc)     execl (rfile, rfile, argv[0], NULL);

    execl (rfile, rfile, NULL);
    _exit (1);
  }

  else {
    /* Encode the open files into a two character string */
    char ft[3];
    char rp[MU_BUF + 1]; /* rogue pid, +1 for paranoia */

    /* zeroize arrays */
    memset (rp, 0, sizeof(rp)); /* paranoia */

    ft[0] = 'a' + frogue;
    ft[1] = 'a' + trogue;
    ft[2] = '\0';

    /* Pass the process ID of the Rogue process as an ASCII string */
    snprintf (rp, MU_BUF, "%d", child);

    execl (pfile, pfile, ft, rp, options, roguename, NULL);
    fprintf (stderr, "ERROR: Rogomatic not available, player binary missing: %s\n", pfile);
    kill (child, SIGKILL);
  }
}

/*
 * replaylog: Given a log file name and an options string, exec the player
 * process to replay the game.  No Rogue process is needed (since we are
 * replaying an old game), so the frogue and trogue file descrptiors are
 * given the fake value 'Z'.
 */

static void
replaylog (char *pfile, char *fname, char *options)
{
  execl (pfile, pfile, "ZZ", "0", options, fname, NULL);
  fprintf (stderr, "ERROR: Replay not available, player binary missing: %s\n", pfile);
  exit (1);
}
