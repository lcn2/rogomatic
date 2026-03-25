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
# include <errno.h>
# include <time.h>
# include <sys/time.h>

# include "types.h"
# include "install.h"

# define READ    0
# define WRITE   1

# define VERSION "14.1.2 20026-03-21"

/*
 * global declarations
 */
char rgmdir[MU_BUF + 1];	/* rogomatic directory - may include UTC date and time sub-dir, +1 for paranoia */
char lock_path[TY_BUF + 1];	/* rogomatic lock file path, +1 for paranoia */

/*
 * static declarations
 */
static const char * const usage =
  "usage: %s [-h] [-V] [-c] [-e] [-f rogue] [-H] [-p] [-r] [-s] [-S ROGOSEED] [-t] [-u] [-w] [-E]\n"
  "\n"
  "    -h            print help message and exit\n"
  "    -V            print version string and exit\n"
  "\n"
  "    -c            use trap arrows\n"
  "    -d            use unique directory name\n"
  "    -e            echo file to roguelog\n"
  "    -f rogue      set path to rogue\n"
  "    -H            disable \"halftime\" show\n"
  "    -p            play back roguelog\n"
  "    -r            use saved game\n"
  "    -s            print scores only\n"
  "    -S ROGOSEED   set $ROGOSEED environment variable for rogue\n"
  "    -t            give status lines only\n"
  "    -u            start up in user mode\n"
  "    -w            set watched mode\n"
  "    -E            set emacs mode\n"
  "\n"
  "Exit codes:\n"
  "    0         all OK\n"
  "    1         some error occured\n"
  "    2         -h and help string printed or -V and version string printed\n"
  "    3         command line error\n"
  " >= 10        internal error\n"
  "\n"
  "%s version: %s\n";

static int   frogue, trogue;

static void replaylog (char *pname, char *fname, char *options);

int
main (int argc, char *argv[])
{
  int ptc[2], ctp[2];
  bool cheat = false;		    /* true ==> Will use trap arrows */
  bool time_subpath = false;	    /* true ==> uses UTC date and time sub-directory */
  bool echo = false;		    /* true ==> Echo file to roguelog */
  bool nohalf = false;		    /* true ==> No halftime show */
  bool replay = false;		    /* true ==> Play back roguelog */
  bool oldgame = false;		    /* true ==> Use saved game */
  bool score = false;		    /* true ==> Give scores only */
  bool terse = false;		    /* true ==> Give status lines only */
  bool user = false;		    /* true ==> Start up in user mode */
  bool noterm = false;		    /* true ==> Watched mode */
  bool emacs = false;		    /* true ==> Emacs mode */
  int child;
  char *rfile = "", *rfilearg = "";
  char options[MU_BUF + 1];	    /* rogomatic options, +1 for paranoia */
  char ropts[SM_BUF + 1];	    /* rogue options, +1 for paranoia */
  char roguename[MU_BUF + 1];	    /* rogue player name, +1 for paranoia */
  char *pfile = "";
  char *program = "";		    /* our name */
  char *prog = "";		    /* basename of our name */
  char *rogue_savefile = NULL;	    /* the rogue save file to restore */
  char rogoseed[MU_BUF + 1];	    /* dungeon number to set as a unsigned seed */
  extern char *optarg;		    /* option argument */
  extern int optind;		    /* argv index of the next arg */
  int i;

  /*
   * set our program and prog basename
   */
  program = argv[0];
  prog = rindex(program, '/');
  if (prog == NULL) {
    prog = program;
  } else {
    ++prog;
  }

  /* zeroize arrays */
  memset (options, 0, sizeof(options)); /* paranoia */
  memset (ropts, 0, sizeof(ropts)); /* paranoia */
  memset (roguename, 0, sizeof(roguename)); /* paranoia */
  memset (rgmdir, 0, sizeof(rgmdir)); /* paranoia */
  memset (lock_path, 0, sizeof(lock_path)); /* paranoia */
  memset (rogoseed, 0, sizeof(rogoseed)); /* paranoia */

  /* initialize rogomatic directory path to default */
  strncpy (rgmdir, RGMDIR, sizeof(rgmdir)-1);

  /*
   * parse args
   */
  while ((i = getopt (argc, argv, "hVcdD:ef:HprsS:tuwE:")) != -1) {
    switch (i) {
      case 'h':		/* -h ==> print usage message */
	fprintf (stderr, usage, program, prog, VERSION);
	exit (2);
	break;

      case 'V':		/* -V -==> print version string and exit */
	printf ("%s\n", VERSION);
	exit (2);
	break;

      case 'c':		/* -c ==> Will use trap arrows! */
	cheat = true;
	break;

      case 'd':		/* -d ==> player uses UTC date and time sub-directory under rogomatic directory path */
	time_subpath = true;
	break;

      case 'D':		/* -D path ==> set the rogomatic directory path */
	memset (rgmdir, 0, sizeof(rgmdir));
	strncpy (rgmdir, optarg, sizeof(rgmdir)-1);
	break;

      case 'e':		/* -e ==> Echo file to roguelog */
	echo = true;
	break;

      case 'f':		/* -f rogue ==> set path to rogue */
	rfilearg = optarg;
	break;

      case 'H':		/* -H ==> No halftime show */
	nohalf = true;
	break;

      case 'p':		/* -p ==> Play back roguelog */
	replay = true;
	break;

      case 'r':		/* -r ==> Use saved game */
	oldgame = true;
	break;

      case 's':		/* -s ==> Give scores only */
	score = true;
	break;

      case 'S':
	strncpy(rogoseed, optarg, sizeof(rogoseed));
	break;

      case 't':		/* -t ==> Give status lines only */
	terse = true;
	break;

      case 'u':		/* -u ==> Start up in user mode */
	user = true;
	break;

      case 'w':		/* -w ==> Watched mode */
	noterm = true;
	break;

      case 'E':		/* -E ==> Emacs mode */
	emacs = true;
	break;

      case ':':
	fprintf(stderr, "%s: ERROR: requires an argument -- %c\n", program, optopt);
	fprintf (stderr, usage, program, prog, VERSION);
	exit (3);
	break;

      case '?':
	fprintf(stderr, "%s: ERROR: unknown option -- %c\n", program, optopt);
	fprintf (stderr, usage, program, prog, VERSION);
	exit (3);
	break;

      default:
	fprintf(stderr, "%s: ERROR: invalid -flag\n", program);
	fprintf (stderr, usage, program, prog, VERSION);
	exit (3);
	break;
    }
  }
  /* skip over command line options */
  argv += optind;
  argc -= optind;

  /*
   * set ROGOSEED environment variable
   *
   * For rogue, because we will prefix the rogue player name with "rogo-", the
   * $ROGOSEED environment variable will determine the rogue dungeon number.
   * For player, the value of the $ROGOSEED environment variable will be recorded
   * in the "pidlog" file under the rogomatic directory.
   */
  if (rogoseed[0] == '\0') {
    struct timeval tp;	/* now */
    unsigned int dnum;	/* dungeon number to set */

    /*
     * determine dungeon number
     */
    if (gettimeofday (&tp, NULL) < 0) {
      dnum = time(NULL);
    } else {
      dnum = ((unsigned int)(tp.tv_sec) ^ (((unsigned int)tp.tv_usec) << 12));
    }
    dnum += (unsigned int) getpid();
    dnum += (unsigned int) getuid();

    /*
     * convert dungeon number into a string
     */
    snprintf (rogoseed, sizeof (rogoseed)-1, "%u", dnum);
  }
  if (setenv ("ROGOSEED", rogoseed, 1) != 0) {
    fprintf (stderr, "ERROR: can't setenv (\"ROGOSEED\", \"%s\", 1)\n", rogoseed);
    exit (1);
  }

  /*
   * determine the rogomatic directory path and rogomatic lock file path
   *
   * However, do not form the UTC date and time sub-directory just yet.
   * Let the player command do that so that the UTC date and time
   * is of when the player command starts.
   */
  set_rgmdir (false);

  /*
   * Find which rogue executable to use
   */
  if (*rfilearg) {
    if (access (rfilearg, R_OK|X_OK) == 0) {
	rfile = rfilearg;
    }
    else {
	fprintf (stderr, "ERROR: rogue arg not executable: %s - %s\n", rfilearg, strerror (errno));
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

  snprintf (options, MU_BUF, "%d,%d,%d,%d,%d,%d,%d,%d",
           cheat, noterm, echo, nohalf, emacs, terse, user, time_subpath);
  snprintf (roguename, MU_BUF, "Rog-O-Matic %s for %s", RGMVER, getname ());
  /* NOTE: The rogue save, rogue score, and rogue lock files are NOT subject to the -d (UTC date and time sub-dir */
  snprintf (ropts, SM_BUF, "%s,%s,%s,%s,%s,%s,inven=%s,name=%s,fruit=%s,file=%s/%s,score=%s/%s,lock=%s/%s",
	    "terse", "noflush", "jump", "seefloor", "nopassgo", "tombstone", "slow", getname (), "apricot",
	    rgmdir, "rogue.sav", rgmdir, "rogue.scr", rgmdir, "rogue.lck");

  if (score)  { dumpscore (argc==1 ? argv[0] : DEFVER); exit (0); }

  if (replay) { replaylog (pfile, argc==1 ? argv[0] : ROGUELOG, options); exit (0); }

  if ((pipe (ptc) < 0) || (pipe (ctp) < 0)) {
    fprintf (stderr, "ERROR: Cannot get pipes!\n");
    exit (1);
  }

  trogue = ptc[WRITE];
  frogue = ctp[READ];

  child = fork ();
  if (child == 0) {

    /*
     * child process that will become rogue
     */

    /* dup child pipe side into stdin and stdout */
    dup2 (ptc[READ], STDIN_FILENO);
    dup2 (ctp[WRITE], STDOUT_FILENO);

    /*
     * set vt100 terminal as player can parse vt100 terminal output
     */
    if (setenv ("TERM", "vt100", 1) != 0) {
      fprintf (stderr, "ERROR: can't setenv (\"TERM\", \"vt100\", 1)\n");
      exit (1);
    }

    /*
     * set rogomatic options for player to use
     */
    if (setenv ("ROGUEOPTS", ropts, 1) != 0) {
      fprintf (stderr, "ERROR: can't setenv (\"ROGUEOPTS\", \"%s\", 1)\n",ropts);
      exit (1);
    }

    /* close down pipe sides used by parent process */
    close (ptc[WRITE]);
    close (ctp[READ]);

    if (oldgame) {
      rogue_savefile = form_path (rgmdir, "rogue.sav");
      execl (rfile, "rogue", "-r", rogue_savefile, NULL);
      fprintf (stderr, "ERROR: rogue default restore exec failed: %s -r %s: %s\n",
		       rfile, rogue_savefile, strerror (errno));
    } else if (argc > 0) {
      execl (rfile, "rogue", argv[0], NULL);
      fprintf (stderr, "ERROR: rogue restore exec failed: %s %s: %s\n", rfile, argv[0], strerror (errno));

    } else {
      execl (rfile, "rogue", NULL);
      fprintf (stderr, "ERROR: rogue exec failed: %s: %s\n", rfile, strerror (errno));
    }
    exit (1);
  }

  else {
    /* Encode the open files into a two character string */
    char ft[3];
    char rp[MU_BUF + 1]; /* rogue pid, +1 for paranoia */

    /*
     * parent process that will become player
     */

    /* zeroize arrays */
    memset (rp, 0, sizeof(rp)); /* paranoia */

    /* let player know the file descriptors that contain the pipes to the rogue program */
    ft[0] = 'a' + frogue;
    ft[1] = 'a' + trogue;
    ft[2] = '\0';

    /* Pass the process ID of the Rogue process as an ASCII string */
    snprintf (rp, MU_BUF, "%d", child);

    /* close down pipe sides used by child rogue process */
    close (ptc[READ]);
    close (ctp[WRITE]);

    execl (pfile, "player", ft, rp, options, roguename, rgmdir, NULL);
    fprintf (stderr, "ERROR: Rogomatic not available, player binary missing: %s\n", pfile);
    kill (child, SIGKILL);
  }
}

/*
 * replaylog: Given a log file name and an options string, exec the player
 * process to replay the game.  No Rogue process is needed (since we are
 * replaying an old game), so the frogue and trogue file descriptors are
 * given the fake value 'Z'.
 */

static void
replaylog (char *pfile, char *fname, char *options)
{
  /* ZZ is the an indicator that player does NOT have a pipe pair to use */
  execl (pfile, "player", "ZZ", "0", options, fname, rgmdir, NULL);
  fprintf (stderr, "ERROR: Replay not available, player binary missing: %s\n", pfile);
  exit (1);
}
