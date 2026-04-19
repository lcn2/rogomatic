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

# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <signal.h>
# include <string.h>
# include <errno.h>
# include <time.h>
# include <sys/time.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "types.h"
# include "config.h"
# include "install.h"

# define READ    0
# define WRITE   1

# define VERSION "14.1.9 2026-04-19"

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
  int child;			    /* fork return: child process id (parent), or 0 (child) */
  char *rfile = NULL;		    /* rogue executable path, or NULL */
  char *rfilearg = NULL;	    /* rogue executable path as specified by -f rogue, or NULL*/
  char options[MU_BUF + 1];	    /* rogomatic options, +1 for paranoia */
  char ropts[SM_BUF + 1];	    /* rogue options, +1 for paranoia */
  char roguename[MU_BUF + 1];	    /* rogue player name, +1 for paranoia */
  char *pfile = NULL;		    /* player path, or NULL */
  char *pfilearg = NULL;	    /* player executable path as specified by -P player, or NULL*/
  char *program = "";		    /* our name */
  char *prog = "";		    /* basename of our name */
  char *rogue_savefile = NULL;	    /* the rogue save file to restore */
  char rogoseed[MU_BUF + 1];	    /* dungeon number to set as a unsigned seed */
  char rogue_dir[MU_BUF + 1];	    /* directory for the rogue files, not impacted by -d */
  unsigned int dnum = 0;	    /* rogue dungeon number to set */
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
  memset (rogue_dir, 0, sizeof(rogue_dir)); /* paranoia */

  /* initialize rogomatic directory path to default */
  strlcpy (rgmdir, RGMDIR, sizeof(rgmdir));

  /* initialize rogue directory path to default */
  strlcpy (rogue_dir, RGMDIR, sizeof(rogue_dir));

  /*
   * parse args
   */
  while ((i = getopt (argc, argv, "hVcdD:ef:HpP:rsS:tuwE:")) != -1) {
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
	memset (rgmdir, 0, sizeof(rgmdir)); /* paranoia */
	strlcpy (rgmdir, optarg, sizeof(rgmdir));
	memset (rogue_dir, 0, sizeof(rogue_dir)); /* paranoia */
	strlcpy (rogue_dir, optarg, sizeof(rogue_dir));
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

      case 'P':		/* -P player ==> change the path of player */
	pfilearg = optarg;
	break;

      case 'r':		/* -r ==> Use saved game */
	oldgame = true;
	break;

      case 's':		/* -s ==> Give scores only */
	score = true;
	break;

      case 'S':		/* -S SEED ==> set the rogomatic seed */
	strlcpy(rogoseed, optarg, sizeof(rogoseed));
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
	fprintf(stderr, "%s: requires an argument -- %c\n", program, optopt);
	fprintf (stderr, usage, program, prog, VERSION);
	exit (3);
	break;

      case '?':
	fprintf(stderr, "%s: unknown option -- %c\n", program, optopt);
	fprintf (stderr, usage, program, prog, VERSION);
	exit (3);
	break;

      default:
	fprintf(stderr, "%s: invalid -flag\n", program);
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
    memset (rogoseed, 0, sizeof(rogoseed)); /* paranoia */
    snprintf (rogoseed, sizeof (rogoseed)-1, "%u", dnum);
  }
  if (rogoseed[0] == '\0') {	/* paranoia */
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u rogoseed is an empty string\n",
		     __func__, __FILE__, __LINE__, dnum);
    exit (1);
  }
  if (setenv ("ROGOSEED", rogoseed, 1) != 0) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u can't setenv (\"ROGOSEED\", \"%s\", 1)\n",
		     __func__, __FILE__, __LINE__, dnum, rogoseed);
    exit (1);
  }

  /*
   * determine the rogomatic directory path and rogomatic lock file path
   *
   * We will form the UTC date and time sub-directory, modifying rgmdir.
   * When player is execl()-ed, the modified rgmdir as the kast arg.
   */
  set_rgmdir (time_subpath);

  /*
   * save stdin, stdout, and stderr terminal state into saved_termattr file
   */
  if (! save_termattr (rgmdir)) {
    fprintf (stderr, "Warning: terminal attributes will NOT be restored later on\n");
  }

  /*
   * Find which rogue executable to use
   */
  if (rfilearg != NULL) {
    if (access (rfilearg, R_OK|X_OK) == 0) {
	rfile = rfilearg;
    }
    else {
	fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u rogue arg not executable: %s - %s\n",
			 __func__, __FILE__, __LINE__, dnum, rfilearg, strerror (errno));
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
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u rogue not found\n",
		      __func__, __FILE__, __LINE__, dnum);
    exit (1);
  }

  /*
   * Find which player executable to use
   */
  if (pfilearg != NULL) {
    if (access (pfilearg, R_OK|X_OK) == 0) {
	pfile = pfilearg;
    }
    else {
	fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u player arg not executable: %s - %s\n",
			 __func__, __FILE__, __LINE__, dnum, pfilearg, strerror (errno));
	exit (1);
    }
  }
  else if (access ("./player", R_OK|X_OK) == 0) {
      pfile = "./player";
  }
# ifdef PLAYER
  else if (access (PLAYER, R_OK|X_OK) == 0) {
      pfile = PLAYER;
  }
# endif
  else {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u player not found\n",
		      __func__, __FILE__, __LINE__, dnum);
    exit (1);
  }

  /*
   * setup values that will be used as arguments to player
   */
  snprintf (options, MU_BUF, "%d,%d,%d,%d,%d,%d,%d,%u",
           cheat, noterm, echo, nohalf, emacs, terse, user, dnum);
  snprintf (roguename, MU_BUF, "Rog-O-Matic %s for %s", RGMVER, getname ());
  /* NOTE: The rogue save, rogue score, and rogue lock files are NOT subject to the -d (UTC date and time sub-dir */
  snprintf (ropts, SM_BUF, "%s,%s,%s,%s,%s,%s,inven=%s,name=%s,fruit=%s,file=%s/%s,score=%s/%s,lock=%s/%s",
	    "terse", "noflush", "jump", "seefloor", "nopassgo", "tombstone", "slow", getname (), "apricot",
	    rogue_dir, "rogue.sav", rogue_dir, "rogue.scr", rogue_dir, "rogue.lck");

  /*
   * special execution case: dumping rogomatic score
   */
  if (score)  {
    dumpscore (argc==1 ? argv[0] : DEFVER);
    exit (0);
  }

  /*
   * special execution case: replay log
   */
  if (replay) {
    if (pfile != NULL) {
      char *fname;	/* log file name */

      /*
       * replaylog: Given a log file name and an options string, exec the player
       * process to replay the game.  No Rogue process is needed (since we are
       * replaying an old game), so the frogue and trogue file descriptors are
       * given the fake value 'Z'.
       *
       * ZZ is the an indicator that player does NOT have a pipe pair to use
       */
      if (argc == 1) {
	fname = argv[0];
      } else {
	fname = ROGUELOG;
      }
      execl (pfile, "player", "ZZ", "0", options, fname, rgmdir, NULL);
      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Replay not available, player binary missing: %s\n",
		       __func__, __FILE__, __LINE__, dnum, pfile);
    } else {
      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u replay: true, pfile == NULL\n",
		       __func__, __FILE__, __LINE__, dnum);
    }
    exit(1);

  }

  /*
   * setup pipes between rogue and player
   */
  if ((pipe (ptc) < 0) || (pipe (ctp) < 0)) {
    fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Cannot get pipes!\n",
		     __func__, __FILE__, __LINE__, dnum);
    exit (1);
  }
  trogue = ptc[WRITE];
  frogue = ctp[READ];

  /*
   * fork rogue child
   */
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
      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u can't setenv (\"TERM\", \"vt100\", 1)\n",
		       __func__, __FILE__, __LINE__, dnum);
      exit (1);
    }

    /*
     * set rogomatic options for player to use
     */
    if (setenv ("ROGUEOPTS", ropts, 1) != 0) {
      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u can't setenv (\"ROGUEOPTS\", \"%s\", 1)\n",
		       __func__, __FILE__, __LINE__, dnum, ropts);
      exit (1);
    }

    /* close down pipe sides used by parent process */
    close (ptc[WRITE]);
    close (ctp[READ]);

    /*
     * exec a rogue game with the args as needed
     */
    if (oldgame) {

      if (rfile != NULL) {
	rogue_savefile = form_path (rgmdir, "rogue.sav");
	execl (rfile, "rogue", "-r", rogue_savefile, NULL);
	fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u rogue default restore exec failed: %s -r %s: %s\n",
			 __func__, __FILE__, __LINE__, dnum, rfile, rogue_savefile, strerror (errno));
      } else {
	fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u oldgame: true, rogue path is NULL\n",
		         __func__, __FILE__, __LINE__, dnum);
      }

    } else if (argc > 0) {

      if (rfile != NULL) {
	execl (rfile, "rogue", argv[0], NULL);
	fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u rogue restore exec failed: %s %s: %s\n",
			 __func__, __FILE__, __LINE__, dnum, rfile, argv[0], strerror (errno));
      } else {
	fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u argc: %d > 0, rogue path is NULL\n",
			 __func__, __FILE__, __LINE__, dnum, argc);
      }

    } else if (rfile != NULL) {

      execl (rfile, "rogue", NULL);
      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u rogue exec failed: %s: %s\n",
		       __func__, __FILE__, __LINE__, dnum, rfile, strerror (errno));

    } else {

      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u rogue path never set, rogue path is NULL\n",
		       __func__, __FILE__, __LINE__, dnum);

    }
    exit (1);
  }

  /*
   * execute player as parent process
   *
   * XXX - Instead of execl(2) of player, we should start off running code along the lines of main.c,
   *	   and fork a child (with pipes) to run rogue.  The global variables that are used in main.c
   *	   should instead of a function that re-initializes those global variables that need to be
   *	   re-initialized.  The SIGCHLD received should trigger whatever "end of rogue" game that
   *	   is needed, and when have the option re-spawning a new rogue game.  We don't need a separate
   *	   player executable, just a rogomatic executable that does the proper job of parsing
   *	   command line arg, initializing (or re-initializing those global variables as needed),
   *	   forking a rogue(6) game with pipes, and handling the case where the rogue(6) game exits.
   */
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

    if (pfile != NULL) {
      execl (pfile, "player", ft, rp, options, roguename, rgmdir, NULL);
      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u Rogomatic not available, player binary missing: %s\n",
		       __func__, __FILE__, __LINE__, dnum, pfile);
      kill (child, SIGKILL);
    } else {
      fprintf (stderr, "ERROR: %s: file: %s line: %d dungeon: %u player path never set, pfile is NULL\n",
		       __func__, __FILE__, __LINE__, dnum);
      exit (1);
    }
  }
}
