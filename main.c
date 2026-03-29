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

/*****************************************************************
 *
 * History:     I.    Andrew Appel & Guy Jacobson, 10/81 [created]
 *              II.   Andrew Appel & Guy Jacobson, 1/82  [added search]
 *              III.  Michael Mauldin, 3/82              [added termcap]
 *              IV.   Michael Mauldin, 3/82              [searching]
 *              V.    Michael Mauldin, 4/82              [cheat mode]
 *              VI.   Michael Mauldin, 4/82              [object database]
 *              VII.  All three, 5/82                    [running away]
 *              VIII. Michael Mauldin, 9/82              [improved cheating]
 *              IX.   Michael Mauldin, 10/82             [replaced termcap]
 *              X.    Mauldin, Hamey,  11/82             [Fixes, Rogue 5.2]
 *              XI.   Mauldin,  11/82                    [Fixes, Score lock]
 *              XII.  Hamey, Mauldin,  06/83             [Fixes, New Replay]
 *              XIII. Mauldin, Hamey,  11/83             [Fixes, Rogue 5.3]
 *              XIV.  Mauldin          01/85             [Fixes, UT mods]
 *              0.0.0 Anthony Molinaro 03/2008           [Restored]
 *
 * General:
 *
 * This is the main routine for the player process, which decodes the
 * Rogue output and sends commands back. This process is execl'd by the
 * rogomatic process (cf. setup.c) which also execl's the Rogue process,
 * conveniently connecting the two via two pipes.
 *
 * Source Files:
 *
 *      arms.c          Armor, Weapon, and Ring handling functions
 *      command.c       Effector interface, sends cmds to Rogue
 *      database.c      Memory for objects "discovered"
 *      debug.c         Contains the debugging functions
 *      explore.c       Path searching functions, exploration
 *      findscore.c     Reads Rogue scoreboard
 *      io.c            I/O functions, Sensory interface
 *      main.c          Main Program for 'player' (this file)
 *      mess.c          Handles messages from Rogue
 *      monsters.c      Monster handling utilities
 *      mover.c         Creates command strings to accomplish moves
 *      rooms.c         Room specific functions, new levels
 *      scorefile.c     Score file handling utilities
 *      search.c        Does shortest path
 *      setup.c         Main program for 'rogomatic'
 *      strategy.c      Makes high level decisions
 *      survival.c      Find cycles and places to run to
 *      tactics.c       Medium level intelligence
 *      things.c        Builds commands, part of Effector interface
 *      titlepage.c     Prints the animated copyright notice
 *      utility.c       Miscellaneous Unix (tm) functions
 *      worth.c         Evaluates the items in the pack
 *
 * Include files:
 *
 *      globals.h       External defs for all global variables
 *      install.h       Machine dependent DEFINES
 *      termtokens.h    Defines various tokens to/from Rogue
 *      types.h         Global DEFINES, macros, and typedefs.
 *
 * Other files which may be included with your distribution include
 *
 *      rplot           A shell script, prints a scatter plot of Rog's scores.
 *      rgmplot.c       A program used by rplot.
 *      histplot.c      A program which plots a histogram of Rgm's scores.
 *
 * Acknowledgments
 *
 *	The UTexas modifications included in this distribution
 *	came from Dan Reynolds, and are included by permission.
 *	Rog-O-Matics first total winner against version 5.3 was
 *	on a UTexas computer.
 *****************************************************************/

# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <ctype.h>
# include <signal.h>
# include <setjmp.h>
# include <string.h>
# include <sys/types.h>
# include <errno.h>
# include <sys/stat.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "modern_curses.h"
# include "types.h"
# include "install.h"
# include "termtokens.h"

/* FIXME: get rid of this prototype in the correct way */
FILE *rogo_openlog (char *genelog);

/* global data - see globals.h for current definitions */

/* Files */
FILE *logfile = NULL;		/* Rogomatic score file */
FILE *realstdout = NULL;	/* Real stdout for Emacs, terse mode */
FILE *snapshot = NULL;		/* File for snapshot command */
FILE *trogue = NULL;		/* Pipe to Rogue process */

/* Characters */
char afterid = '\0';		/* Letter of obj after identify */
char genepool[TY_BUF + 1];	/* Gene pool, +1 for paranoia */
char *genocide;			/* List of monsters to be genocided */
char genocided[MU_BUF + 1];	/* List of monsters genocided, +1 for paranoia */
char lastcmd[MU_BUF + 1];	/* Copy of last command sent to Rogue, +1 for paranoia */
char lastname[NAMSIZ + 1];	/* Name of last potion/scroll/wand, +1 for paranoia */
char nextid = '\0';		/* Next object to identify */
char screen[R][C + 1];		/* Map of current rogue screen - characters drawn by rogue, +1 for paranoia */
char sumline[BIGBUF + 1];	/* Termination message for Rogomatic, +1 for paranoia */
char sumline2[BIGBUF + 1];	/* alternate sumline buffer, +1 for paranoia */
char ourkiller[MU_BUF + 1];	/* What was listed on the tombstone - How we died, +1 for paranoia */
char pending_call_letter = ' ';	/* If non-blank we have a call it to do - Pack object we know a name for */
char pending_call_name[NAMSIZ + 1];	/* Pack object name for letter, +1 for paranoia */
char versionstr[MU_BUF + 1];		/* Version of Rogue being used, +1 for paranoia */
char rgmdir[MU_BUF + 1];		/* rogomatic directory - may include UTC date and time sub-dir, +1 for paranoia */
char lock_path[TY_BUF + 1];	/* rogomatic lock file path, +1 for paranoia */
char roguename[MU_BUF + 1];	/* Name we are playing under, +1 for paranoia */
char *termination = "perditus";	/* Latin verb for how we died */

/* Integers */
bool aggravated = false;	/* True if we have aggravated this level */
int agoalc = NONE;		/* Goal square to arch from (row) */
int agoalr = NONE;		/* Goal square to arch from (col) */
int ammo = 0;			/* Number of missiles in pack */
bool arrowshot = false;		/* True if an arrow shot us last turn */
int atrow = 0;			/* Current position of the rogue (@) (row) */
int atcol = 0;			/* Current position of the rogue (@) (col) */
int atrow0 = 0;			/* Position at start of turn (row) */
int atcol0 = 0;			/* Position at start of turn (col) */
int attempt = 0;		/* Number times we searched whole level */
bool badarrow = false;		/* True if cursed/lousy arrow in hand */
int beingheld = 0;		/* Turns a fungus has held of us */
int beingstalked = 0;		/* Invisible stalker strategies */
bool blinded = false;		/* True if blinded */
int blindir = 0;		/* Last direction we moved when blind */
int cancelled = 0;		/* Turns till use cancellation again */
bool cheat = false;		/* True ==> cheat, use bugs, etc. */
bool checkrange = false;	/* True ==> check range */
bool chicken = false;		/* True ==> check run away code */
bool compression = true;	/* True ==> move more than one square/turn */
bool confused = false;		/* True if we are confused */
bool cosmic = false;		/* True if we are hallucinating */
int currentarmor = NONE;	/* Index of our armor */
int currentweapon = NONE;	/* Index of our weapon */
bool cursedarmor = false;	/* True if our armor is cursed */
bool cursedweapon = false;	/* True if we are wielding cursed weapon */
int darkdir = NONE;		/* Direction of arrow in dark room */
int darkturns = 0;		/* Number of arrows left to fire in dark room */
int debugging = D_NORMAL;	/* Debugging options in effect */
int didreadmap = 0;		/* Last level we read a map on */
int doorlist[40];		/* Holds row or column of new doors found on this level */
bool doublehasted = false;	/* True if double hasted (Rogue 3.6) */
int droppedscare = 0;		/* Number of scare mon. on this level */
bool diddrop = false;		/* True if we dropped anything on this spot */
bool emacs = false;		/* True ==> format output for Emacs */
bool exploredlevel = false;	/* True if we completely explored this level */
bool floating = false;		/* True if we are levitating */
int foughtmonster = 0;		/* rounds we fought a monster */
bool foundarrowtrap = false;	/* Found arrow trap this level */
bool foundtrapdoor = false;	/* Found trap door this level */
int goalr = NONE;		/* Current goal square (row) */
int goalc = NONE;		/* Current goal square (col) */
int goodarrow = 0;		/* Number of times we killed in one blow */
bool goodweapon = false;	/* True if weapon in hand worth >= 100 */
int gplusdam = 1;		/* Our plus damage bonus from strength */
int gplushit = 0;		/* Our plus to hit bonus from strength */
bool hasted = false;		/* True if hasted */
int head = 0;			/* starting index of circular queue */
int tail = 0;			/* ending index of circular queue */
int hitstokill = 0;		/* Number of hits to kill last monster */
bool interrupted = false;	/* True if at commandtop from onintr() */
bool knowident = false;		/* True if found an identify scroll */
int larder = 1;			/* Number of foods left */
int lastate = 0;		/* Time we last ate */
int lastdamage = 0;		/* Amount of last hit by a monster */
int lastdrop = NONE;		/* Last object we tried to drop */
int lastfoodlevel = 1;		/* Last level we found food */
int lastmonster = NONE;		/* Last monster we tried to hit */
int lastobj = NONE;		/* What did we last try to use */
int lastwand = NONE;		/* Index of last wand */
int leftring = NONE;		/* Index of our left ring */
bool logdigested = false;	/* True if log file has been read by replay */
bool logging = false;		/* True if keeping record of game */
bool lyinginwait = false;	/* True if we waited for a monster */
int maxobj = 22;		/* How much can we carry */
bool missedstairs = false;	/* True if we searched everywhere */
int morecount = 0;		/* Number of messages since last command */
bool msgonscreen = false;	/* True if there is a rogomatic msg on the screen */
bool newarmor = true;		/* True if our armor status has changed */
int *newdoors = NULL;		/* pointer to a row or column of a new door found on this level */
bool newring = true;		/* True if our ring status has changed */
bool newweapon = true;		/* True if our armor status has changed */
bool nohalf = false;		/* True if no halftime show */
bool noterm = false;		/* True if no human watching */
int objcount = 0;		/* Number of objects */
int ourscore = 0;		/* Our score when we died/quit */
bool playing = true;		/* True if still playing game */
bool poorarrow = false;		/* True if arrow has missed */
bool protected = false;		/* True if we protected our armor */
int putonseeinv = 0;		/* Turn when last put on see inv ring */
bool redhands = false;		/* True if we have red hands */
bool replaying = false;		/* True if replaying old game */
bool revvideo = false;		/* True if in rev. video mode */
int rightring = NONE;		/* Index of our right ring */
int rogpid = -1;		/* Pid of rogue process */
int room[RGRID + 1];		/* Flags for each room, +! for paranoia */
int row = 0;			/* Current cursor position (row) */
int col = 0;			/* Current cursor position (col) */
int scrmap[R][C + 1];		/* Flags bits for level map, +1 for paranoia */
int slowed = 0;			/* turns since we slowed a monster */
int stairrow = 0;		/* Position of stairs on this level (row) */
int staircol = 0;		/* Position of stairs on this level (col) */
int teleported = 0;		/* Number of times teleported this level */
bool terse = false;		/* True if in terse mode */
bool transparent = false;	/* True if in user command mode */
int trapr = NONE;		/* Location of arrow trap, this level (row) */
int trapc = NONE;		/* Location of arrow trap, this level (col) */
int urocnt = 0;			/* Un-identified Rogue Object count */
bool usesynch = false;		/* True when the inventory is correct - finished using something */
bool usingarrow = false;	/* True if wielding an arrow from a trap */
int version = 0;		/* rogue version is an integer as set by getrougeversion() */
int wplusdam = 2;		/* Our plus damage from weapon bonus */
int wplushit = 1;		/* Our plus hit from weapon bonus */
int zone = NONE;		/* Current screen zone, 0..8 */
int zonemap[RGRID][RGRID + 1];	/* Connectivity map - Map of zones connections, +1 for paranoia */

/* Functions */
void (*istat)(int);

/* Stuff list, list of objects on this level */
stuffrec slist[MAXSTUFF + 1];	/* +1 for paranoia */
int slistlen = 0;		/* count of objects in slist[] */

/* Monster list, list of monsters on this level */
monrec mlist[MAXMONST + 1];	/* +1 for paranoia */
int mlistlen = 0;		/* count of monsters in mlist[] */

char targetmonster = '@';	/* Monster we are arching at */

/* Monster attribute and Long term memory arrays */
attrec monatt[Z + 1];		/* Monster attributes, +1 for paranoia */
lrnrec ltm;			/* Long term memory -- general */
ltmrec monhist[MAXMON + 1];	/* Long term memory -- creatures, +1 for paranoia */
int nextmon = 0;		/* Length of LTM */
int monindex[MAXMONST + 1];	/* Index into monhist array, +1 for paranoia */

/* Genetic learning parameters (and defaults) */
int geneid = 0;		/* Id of genotype */
int genebest = 0;	/* Best score of genotype */
int geneavg = 0;	/* Average score of genotype */
int k_srch =	50;	/* Propensity for searching for traps */
int k_door =	50;	/* Propensity for searching for doors */
int k_rest =	50;	/* Propensity for resting */
int k_arch =	50;	/* Propensity for firing arrows */
int k_exper =	50;	/* Level*10 on which to experiment with items */
int k_run =	50;	/* Propensity for retreating */
int k_wake =	50;	/* Propensity for waking things up */
int k_food =	50;	/* Propensity for hoarding food (affects rings) */
static int knob[MAXKNOB] = {50, 50, 50, 50, 50, 50, 50, 50};	/* Knobs */
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
/* Door search map */
char timessearched[R][C + 1]; /* +1 for paranoia */
char timestosearch = '\0';
int searchstartr = NONE;
int searchstartc = NONE;
int reusepsd = 0;
bool new_mark = true;
bool new_findroom = true;
bool new_search = true;
bool new_stairs = true;
bool new_arch = true;

/* Results of last call to makemove() */
int  ontarget = 0;
int  targetrow = NONE;
int  targetcol = NONE;

/* Rog-O-Matics model of his stats */
int   Level = 0;
int   MaxLevel = 0;
int   Gold = 0;
int   Hp = 12;
int   Hpmax = 12;
int   Str = 16;
int   Strmax = 16;
int   Ac = 6;
int   Exp = 0;
int   Explev = 1;
int   turns = 0;
char  Ms[30];	/* The message about his state of hunger */

/* Miscellaneous movement tables, +1 for paranoia */
int   deltrc[DNUM + 1] = { 1, -(C-1), -C, -(C+1), -1, C-1, C, C+1, 0 };
int   deltc[DNUM + 1]  = { 1, 1, 0, -1, -1, -1, 0, 1, 0 };
int   deltr[DNUM + 1]  = { 0, -1, -1, -1, 0, 1, 1, 1, 0 };
char  keydir[DNUM + 1] = { 'l', 'u', 'k', 'y', 'h', 'b', 'j', 'n', '\0' };
int   movedir = 0;

/* Map characters on screen into object types */
stuff translate[128] = {
  /* \00x */  none, none, none, none, none, none, none, none,
  /* \01x */ none, none, none, none, none, none, none, none,
  /* \02x */ none, none, none, none, none, none, none, none,
  /* \03x */ none, none, none, none, none, none, none, none,
  /* \04x */ none, potion, none, none, none, none, none, none,
  /* \05x */ hitter, hitter, gold, none, amulet, none, none, wand,
  /* \06x */ none, none, none, none, none, none, none, none,
  /* \07x */ none, none, food, none, none, ring, none, Scroll,
  /* \10x */ none, none, none, none, none, none, none, none,
  /* \11x */ none, none, none, none, none, none, none, none,
  /* \12x */ none, none, none, none, none, none, none, none,
  /* \13x */ none, none, none, armor, none, armor, none, none,
  /* \14x */ none, none, none, none, none, none, none, none,
  /* \15x */ none, none, none, none, none, none, none, none,
  /* \16x */ none, none, none, none, none, none, none, none,
  /* \17x */ none, none, none, none, none, none, none, none
};

/* Inventory, contents of our pack */
char space[MAXINV][NAMSIZ + 1]; /* inventory string space, +1 for paranoia */
invrec inven[MAXINV + 1];   /* +1 for paranoia */
int invcount = 0;

/* Time history */
timerec timespent[50];

/* End of the game messages */
char *gamename = "Rog-O-Matic";

/* Used by onintr() to restart Rgm at top of command loop */
jmp_buf  commandtop;

/* static storage */
static char genelock[TY_BUF + 1];	/* Gene pool lock file, +1 for paranoia */
static char genelog[TY_BUF + 1];	/* Genetic learning log file, +1 for paranoia */

/* static function declarations */
static void onintr (int sig);
static void startlesson (void);
static void endlesson (void);

/*
 * Main program
 */

int
main (int argc, char *argv[])
{
  char  ch, *s;
  char msg[SM_BUF + 1];		/* message buffer, +1 for paranoia */
  bool singlestep = false;	/* True ==> go one turn */
  bool startecho = false;	/* True ==> turn on echoing on startup */
  bool startingup = true;	/* True ==> startup started but not complete */
  bool time_subpath = false;	/* True ==> use a UTC date and time sub-directory */
  char logfilename[100];	/* Name of log file */
  pid_t pid = -1;		/* process id */
  char pidfilename[TY_BUF + 1]; /* +1 for paranoia */
  FILE *pidfp = NULL;		/* open pidfilename stream */
  int i;

  /*
   * Initialize some storage
   *
   * zeroize arrays and other initializations
   */

  /* zeroize arrays */
  memset (genepool, 0, sizeof(genepool)); /* paranoia */
  memset (genocided, 0, sizeof(genocided)); /* paranoia */
  memset (lastcmd, 0, sizeof(lastcmd)); /* paranoia */
  lastcmd[0] = 'i';
  memset (lastname, 0, sizeof(lastname)); /* paranoia */
  for (i=0; i < R; ++i) {
      memset (&(screen[i][0]), ' ', sizeof(screen[i])); /* screen lines initialize with ASCII space */
      screen[i][C] = '\0'; /* paranoia */
  }
  memset (sumline, 0, sizeof(sumline)); /* paranoia */
  memset (sumline2, 0, sizeof(sumline2)); /* paranoia */
  memset (ourkiller, 0, sizeof(ourkiller)); /* paranoia */
  strlcpy (ourkiller, "unknown", sizeof(ourkiller));
  memset (pending_call_name, 0, sizeof(pending_call_name)); /* paranoia */
  memset (versionstr, 0, sizeof(versionstr)); /* paranoia */
  strlcpy (versionstr, DEFVER, sizeof(versionstr));
  memset (rgmdir, 0, sizeof(rgmdir)); /* paranoia */
  memset (lock_path, 0, sizeof(lock_path)); /* paranoia */
  memset (roguename, 0, sizeof(roguename)); /* paranoia */
  /**/
  memset (room, 0, sizeof(room)); /* paranoia */
  memset (scrmap, 0, sizeof(scrmap)); /* paranoia */
  memset (zonemap, 0, sizeof(zonemap)); /* paranoia */
  memset (slist, 0, sizeof(slist)); /* paranoia */
  memset (mlist, 0, sizeof(mlist)); /* paranoia */
  memset (monatt, 0, sizeof(monatt)); /* paranoia */
  memset (&ltm, 0, sizeof(ltm)); /* paranoia */
  memset (monhist, 0, sizeof(monhist)); /* paranoia */
  memset (monindex, 0, sizeof(monindex)); /* paranoia */
  memset (timessearched, 0, sizeof(timessearched)); /* paranoia */
  memset (Ms, 0, sizeof(Ms)); /* paranoia */
  memset (space, 0, sizeof(space)); /* paranoia */
  memset (inven, 0, sizeof(inven)); /* paranoia */
  doresetinv (); /* reset the inventory */
  memset (timespent, 0, sizeof(timespent)); /* paranoia */
  memset (commandtop, 0, sizeof(commandtop)); /* paranoia */

  /* zeroize static storage */
  memset (genelock, 0, sizeof(genelock)); /* paranoia */
  memset (genelog, 0, sizeof(genelog)); /* paranoia */

  /* zeroize local arrays */
  memset (msg, 0, sizeof(msg)); /* paranoia */
  memset (logfilename, 0, sizeof(logfilename)); /* paranoia */
  memset (pidfilename, 0, sizeof(pidfilename)); /* paranoia */

  /*
   * on exit: cleanup I/O, and shutdown ncurses (if needed)
   */
  (void) atexit(endwin_and_ncurses_cleanup);

  /* The second argument to player is the process id of Rogue */
  errno = 0;
  if (argc > 2) {
      rogpid = atoi (argv[2]);
      if (errno != 0) {
	  fprintf (stderr, "ERROR: argv[2]: %s cannot be converted into an int: %s\n", argv[2], strerror (errno));
	  exit(1);
      }
      if (rogpid < 0) {
	  fprintf (stderr, "ERROR: argv[2]: %s pid arg < 0: %d\n", argv[2], rogpid);
	  exit(1);
      }
  }

  /* The third argument is an option list */
  if (argc > 3) {
      int cheat_int;		/* integer form of cheat boolean */
      int noterm_int;		/* integer form of noterm boolean */
      int startecho_int;	/* integer form of startecho boolean */
      int nohalf_int;		/* integer form of nohalf boolean */
      int emacs_int;		/* integer form of emacs boolean */
      int terse_int;		/* integer form of terse boolean */
      int transparent_int;	/* integer form of transparent boolean */
      int time_subpath_int;	/* integer form of time_subpath boolean */

      /* parse options in third argument */
      i = sscanf (argv[3], "%d,%d,%d,%d,%d,%d,%d,%d",
                            &cheat_int, &noterm_int, &startecho_int, &nohalf_int,
                            &emacs_int, &terse_int, &transparent_int, &time_subpath_int);
      if (i != 8) {
	  fprintf (stderr, "ERROR: argv[3]: %s failed to scanf 8 flags, returned: %d\n", argv[3], i);
	  exit(1);
      }

      /* convert ints to booleans */
      cheat = (cheat_int == 0) ? false : true;
      noterm = (noterm_int == 0) ? false : true;
      startecho = (startecho_int == 0) ? false : true;
      nohalf = (nohalf_int == 0) ? false : true;
      emacs = (emacs_int == 0) ? false : true;
      terse = (terse_int == 0) ? false : true;
      transparent = (transparent_int == 0) ? false : true;
      time_subpath = (time_subpath_int == 0) ? false : true;
  }

  /* The fourth argument is the Rogue name */
  if (argc > 4)	{
      strlcpy (roguename, argv[4], sizeof(roguename));
  } else {
      snprintf (roguename, MU_BUF, "Rog-O-Matic %s", RGMVER);
  }
  roguename[MU_BUF] = '\0'; /* paranoia */

  /* The 5th argument is the non-default rogomatic directory path */
  if (argc > 5) {
    memset (rgmdir, 0, sizeof(rgmdir));
    strlcpy (rgmdir, argv[5], sizeof(rgmdir));
  }

  /*
   * determine the rogomatic directory path and rogomatic lock file path
   */
  set_rgmdir (time_subpath);

  /*
   * send stderr to an errlog file
   */
  redirect_stderr (rgmdir, "errlog");

  /*
   * append rogue pid and $ROGOSEED environment variable to pidlog
   */
  append_pidlog (rgmdir, "pidlog");

  /*
   * The first argument to player is a two character string encoding
   * the file descriptors of the pipe ends. See setup.c for call.
   *
   * If we get 'ZZ', then we are replaying an old game, and there
   * are no pipes to read/write.
   */

  if (argv[1][0] == 'Z') {
    replaying = true;
    gamename = "Iteratum Rog-O-Maticus";
    termination = "finis";
    strlcpy (logfilename, argv[4], sizeof(logfilename));
    startreplay (&logfile, logfilename);
  }
  else {
    int frogue_fd = argv[1][0] - 'a';
    int trogue_fd = argv[1][1] - 'a';
    open_frogue_fd (frogue_fd);
    open_frogue_debuglog (rgmdir, "debuglog.frogue");
    trogue = fdopen (trogue_fd, "w");
    setbuf (trogue, NULL);
  }

  /*
   * open debug logging
   *
   * Get the process id of this player program if the
   * environment variable is set which requests this be
   * done.  Then create the file name with the PID so
   * that the debugging scripts can find it and use the
   * PID.
   *
   * This code can be removed if you don't need to use
   * the debugging scripts.
   *
   */

  if (getenv("GETROGOMATICPID") != NULL) {
    pid = getpid ();
    memset (pidfilename, 0, sizeof(pidfilename));
    snprintf (pidfilename, sizeof(pidfilename)-1, "%s/rogomaticpid.%d", rgmdir, pid);
    if ((pidfp = fopen (pidfilename, "w")) == NULL) {
      fprintf (stderr, "ERROR: Can't open '%s'.\n", pidfilename);
      exit(1);
    }
  }
  debuglog_open (rgmdir, "debuglog.player");

  /* If we are in one-line mode, then squirrel away stdout */
  if (emacs || terse) {
    realstdout = fdopen (dup (STDOUT_FILENO), "w");
    freopen ("/dev/null", "w", stdout);
  }

  initscr (); crmode (); noecho ();	/* Initialize the Curses package */

  if (startecho) toggleecho ();		/* Start logging? */

  clear ();				/* Clear the screen */
  getrogver ();				/* Figure out Rogue version */

  if (!replaying) {
    restoreltm ();			/* Get long term memory of version */
    startlesson ();			/* Start genetic learning */
  }

  /*
   * Give a hello message
   */

  if (replaying)
    snprintf (msg, SM_BUF, " Replaying log file %s, version %s.",
	     logfilename, versionstr);
  else
    snprintf (msg, SM_BUF, " %s: version %s, genotype %d.",
	     roguename, versionstr, geneid);

  if (emacs)
    { fprintf (realstdout, "%s  (%%b)", msg); fflush (realstdout); }
  else if (terse)
    { fprintf (realstdout, "%s\n", msg); fflush (realstdout); }
  else
    { saynow ("%s", msg); }

  /*
   * Now that we have the version figured out, we can properly
   * interpret the screen.  Force a redraw by sending a redraw
   * screen command (^L for old, ^R for new).
   *
   * Also identify wands (/), so that we can differentiate
   * older Rogue 3.6 from Rogue 3.6 with extra magic...
   */

  if (version < RV53A)
    sendnow ("%c//;", ctrl('l'));
  else
    sendnow ("%c;", ctrl('r'));

  /*
   * If we are not replaying an old game, we must position the
   * input after the next form feed, which signals the start of
   * the level drawing.
   */
  {
    if (!replaying)
      while ((int) (ch = getroguetoken ()) != CL_TOK && (int) ch != EOF) {
        /* FIXME: If you start next to a monster this will get stuck, as
           pressing 'v' takes time in version 3.6, so rogue will be waiting
           for input and we will be waiting for rogue to print a CL_TOK,
           so deadlock - NYM */
      }
  }

  /*
   * Note: If we are replaying, the logfile is now in synch
   */
  getrogue (ILL, 2);  /* Read the input up to end of first command */

  /* Identify all 26 monsters */
  if (!replaying)
    for (ch = 'A'; ch <= 'Z'; ch++) rogo_send ("/%c", ch);

  /*
   * Signal handling. On an interrupt, Rogomatic goes into transparent
   * mode and clears what state information it can. This code is styled
   * after that in "UNIX Programming -- Second Edition" by Brian
   * Kernigan & Dennis Ritchie. I sure wouldn't have thought of it.
   */

  istat = signal (SIGINT, SIG_IGN); /* save original status */
  setjmp (commandtop);              /* save stack position */

  if (istat != SIG_IGN)
    signal (SIGINT, onintr);

  if (interrupted) {
    saynow ("Interrupt [enter command]:");
    interrupted = false;
    transparent = true;
  }

  if (transparent) noterm = false;

  while (playing) {
    refresh ();

    /* If we have any commands to send, send them */
    while (resend ()) {
      if (startingup) showcommand (lastcmd);

      sendnow (";");
      getrogue (ILL, 2);
    }

    if (startingup) {	/* All monsters identified */
      versiondep ();			/* Do version specific things */
      startingup = false;		/* Clear starting flag */
    }

    if (!playing) break;	/* In case we died */

    /*
     * No more stored commands, so either get a command from the
     * user (if we are in transparent mode or the user has typed
     * something), or let the strategize module try its luck. If
     * strategize fails we wait for the user to type something. If
     * there is no user (noterm mode) then use ROGQUIT to signal a
     * quit command.
     */

    if ((transparent && !singlestep) ||
        (!emacs && charsavail ()) ||
        !strategize()) {
      ch = (noterm) ? ROGQUIT : getch ();

      switch (ch) {
        case '?': givehelp (); break;

        case '\n': if (terse)
            { printsnap (realstdout); fflush (realstdout); }
          else
            { singlestep = true; transparent = true; }

          break;

          /* Rogue Command Characters */
        case 'H': case 'J': case 'K': case 'L':
        case 'Y': case 'U': case 'B': case 'N':
        case 'h': case 'j': case 'k': case 'l':
        case 'y': case 'u': case 'b': case 'n':
        case 's': command (T_OTHER, "%c", ch); transparent = true; break;

        case 'f': ch = getch ();

          for (s = "hjklyubnHJKLYUBN"; *s; s++) {
            if (ch == *s) {
              if (version < RV53A) command (T_OTHER, "f%c", ch);
              else                 command (T_OTHER, "%c", ctrl (ch));
            }
          }

          transparent = true; break;

        case '\f':  redrawscreen (); break;

        case 'm':   dumpmonstertable (); break;

        case 'M':   dumpmazedoor (); break;

        case '>': if (atrow == stairrow && atcol == staircol)
            command (T_OTHER, ">");

          transparent = true; break;

        case '<': if (atrow == stairrow && atcol == staircol &&
                        have (amulet) != NONE) command (T_OTHER, "<");

          transparent = true; break;

        case 't': transparent = !transparent; break;

        case ')': new_mark = true; markcycles (DOPRINT); at (row, col); break;

        case '+': setpsd (DOPRINT); at (row, col); break;

        case 'A': attempt = (attempt+1) % 5;
          saynow ("Attempt %d", attempt); break;

        case 'G': mvprintw (0, 0,
                              "%d: Sr %d Dr %d Re %d Ar %d Ex %d Rn %d Wk %d Fd %d, %d/%d",
                              geneid, k_srch, k_door, k_rest, k_arch,
                              k_exper, k_run, k_wake, k_food, genebest, geneavg);
          clrtoeol (); at (row, col); refresh (); break;

        case ':': chicken = !chicken;
          say (chicken ? "chicken" : "aggressive");
          break;

        case '~': if (replaying)
            saynow ("Replaying log file %s, version %s.",
                    logfilename, versionstr);
          else
            saynow (" %s: version %s, genotype %d.",
                    roguename, versionstr, geneid);

          break;

        case '[': at (0,0);
          printw ("%s = %d, %s = %d, %s = %d, %s = %d.",
                  "hitstokill", hitstokill,
                  "goodweapon", goodweapon,
                  "usingarrow", usingarrow,
                  "goodarrow", goodarrow);
          clrtoeol ();
          at (row, col);
          refresh ();
          break;

	case '-': saynow ("%s", statusline ());
          break;

        case '`': clear ();
          summary ((FILE *) NULL, '\n');
          pauserogue ();
          break;

        case '|': clear ();
          timehistory ((FILE *) NULL, '\n');
          pauserogue ();
          break;

        case 'r': resetinv (); say ("Inventory reset."); break;

        case 'i': clear (); dumpinv ((FILE *) NULL); pauserogue (); break;

        case '/': dosnapshot ();
          break;

        case '(': clear (); dumpdatabase (); pauserogue (); break;

        case 'c': cheat = !cheat;
          say (cheat ? "cheating" : "righteous");
          break;

        case 'd': toggledebug ();	break;

        case 'e': toggleecho ();        break;

        case '!': dumpstuff ();         break;

        case '@': dumpmonster ();       break;

        case '#': dumpwalls ();         break;

        case '%': clear (); havearmor (1, DOPRINT, ANY); pauserogue (); break;

        case ']': clear (); havearmor (1, DOPRINT, RUSTPROOF);
          pauserogue (); break;

        case '=': clear (); havering (1, DOPRINT); pauserogue (); break;

        case '$': clear (); haveweapon (1, DOPRINT); pauserogue (); break;

        case '^': clear (); havebow (1, DOPRINT); pauserogue (); break;

        case '{': promptforflags (); break;

        case '&': saynow ("Object count is %d.", objcount); break;

        case '*': blinded = !blinded;
          saynow (blinded ? "blinded" : "sighted");
          break;

        case 'C': cosmic = !cosmic;
          saynow (cosmic ? "cosmic" : "boring");
          break;

        case 'E': dwait (D_ERROR, __func__, "Testing the ERROR trap"); break;

        case 'F': dwait (D_FATAL, __func__, "Testing the FATAL trap"); break;

        case 'R': if (replaying) {
            positionreplay (); getrogue (ILL, 2);

            if (transparent) singlestep = true;
          }
          else
            saynow ("Replay position only works in replay mode.");

          break;

        case 'S': quitrogue ("saved", Gold, SAVED);
          playing = false; break;

        case 'Q': quitrogue ("user typing quit", Gold, FINISHED);
          playing = false; break;

        case ROGQUIT: dwait (D_ERROR, __func__, "Strategize failed: gave up");
          quitrogue ("gave up", Gold, SAVED); break;
      }
    }
    else {
      singlestep = false;
    }
  }

  if (! replaying) {
    saveltm (Gold);			/* Save new long term memory */
    endlesson ();			/* End genetic learning */
  }

  /* Print termination messages */
  at (R-1, 0);
  clrtoeol ();
//  clear ();
  refresh ();
  endwin (); nocrmode (); noraw (); echo ();

  if (emacs) {
    if (*sumline) fprintf (realstdout, " %s", sumline);
  }
  else if (terse) {
    if (*sumline) fprintf (realstdout, "%s\n",sumline);

    fprintf (realstdout, "%s %s est.\n", gamename, termination);
  }
  else {
    if (*sumline) printf ("%s\n",sumline);

    printf ("%s %s est.\n", gamename, termination);
  }

  /*
   * Rename log file, if it is open
   */

  if (logging) {
    char lognam[MU_BUF + 1];	/* log filename, +1 for paranoia */

    /* zeroize arrays */
    memset (lognam, 0, sizeof(lognam)); /* paranoia */

    /* Make up a new log file name */
    snprintf (lognam, MU_BUF, "%.4s.%d.%d", ourkiller, MaxLevel, ourscore);

    /* Close the open file */
    toggleecho ();

    /* Rename the log file */
    if (link (ROGUELOG, lognam) == 0) {
      unlink (ROGUELOG);
      printf ("Log file left on %s\n", lognam);
    }
    else
      printf ("Log file left on %s\n", ROGUELOG);
  }

  close_frogue_debuglog ();
  debuglog_close ();
  close_errlog();
  exit (0);
}

/*
 * onintr: The SIGINT handler. Pass interrupts to main loop, setting
 * transparent mode. Also send some synchronization characters to Rogue,
 * and reset some goal variables.
 */

static void
onintr (int sig)
{
  sendnow ("n\033");            /* Tell Rogue we don't want to quit */
  refresh ();                   /* Clear terminal output */
  clearsendqueue ();            /* Clear command queue */
  setnewgoal ();                /* Don't believe ex */
  transparent = true;           /* Drop into transprent mode */
  interrupted = true;           /* Mark as an interrupt */
  noterm = false;               /* Allow commands */
  longjmp (commandtop,0);       /* Back to command Process */
}

/*
 * startlesson: Genetic learning algorithm, pick a genotype to
 * test this game, and set the parameters (or "knobs") accordingly.
 */

static void
startlesson (void)
{
  unsigned int tmpseed = 0;
  int lock_fd;

  snprintf (genelog, sizeof(genelog)-1, "%s/GeneLog%d", rgmdir, version);
  snprintf (genepool, sizeof(genepool)-1, "%s/GenePool%d", rgmdir, version);
  snprintf (genelock, sizeof(genelock)-1, "%s/GeneLock%d", rgmdir, version);

  /* set up random number generation */
  if (getenv("SEED") != NULL) {
    /* if we want repeatable results for testing set
       the environment variable SEED to some positive integer
       value and use a version of rogue that also uses a SEED
       environment variable.  this makes testing so much easier... */
    tmpseed = atoi(getenv("SEED"));
    rogo_srand(tmpseed);
  }
  else
    /* Start random number generator based upon the current time */
    rogo_srand (0);

  critical ();				/* Disable interrupts */

  /* lock */
  lock_fd = lock_file (__func__, NULL, genelock);

  /* Serialize access to the gene pool */
  if (rogo_openlog (genelog) == NULL)	/* Open the gene log file */
    saynow ("Could not open file %s", genelog);

  if (! readgenes (genepool))		/* Read the gene pool */
    initpool (MAXKNOB, 20);		/* Random starting point */

  setknobs (&geneid, knob, &genebest, &geneavg); /* Select a genotype */
  writegenes (genepool);		/* Write out the gene pool */
  rogo_closelog ();			/* Close the gene log file */

  /* unlock */
  unlock_file (__func__, lock_fd);

  uncritical ();			/* Reenable interrupts */

  /* Cache the parameters for easier use */
  k_srch = knob[K_SRCH];	k_door = knob[K_DOOR];
  k_rest = knob[K_REST];	k_arch = knob[K_ARCH];
  k_exper = knob[K_EXPER];	k_run = knob[K_RUN];
  k_wake = knob[K_WAKE];	k_food = knob[K_FOOD];
}

/*
 * endlesson: if killed, total winner, or quit for scoreboard,
 * evaluate the performance of this genotype and save in genepool.
 */

static void
endlesson (void)
{
  int lock_fd;

  if (geneid > 0 &&
      (stlmatch (termination, "perditus") ||
       stlmatch (termination, "victorius") ||
       stlmatch (termination, "callidus"))) {

    critical ();			/* Disable interrupts */

    /* lock */
    lock_fd = lock_file (__func__, NULL, genelock);

    rogo_openlog (genelog);		/* Open the gene log file */

    if (readgenes (genepool)) {	/* Read the gene pool */
      evalknobs (geneid,Gold,Level);	/* Add the trial to the pool */
      writegenes (genepool);
    }	/* Write out the gene pool */

    rogo_closelog ();

    /* unlock */
    unlock_file (__func__, lock_fd);

    uncritical ();			/* Re-enable interrupts */
  }
}
