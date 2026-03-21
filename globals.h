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


#if !defined(INCLUDE_GLOBALS_H)
#define INCLUDE_GLOBALS_H


/*
 * globals.h:
 *
 * Global variables
 */

/* global files */
extern FILE *logfile;		/* Rogomatic score file */
extern FILE *realstdout;	/* Real stdout when in terse or emacs mode */
extern FILE *snapshot;		/* File for snapshot command */
extern FILE *trogue;		/* Pipe to Rogue process */

/* global characters and strings */
extern char afterid;			/* Letter of obj after identify */
extern char genepool[MU_BUF + 1];	/* Gene pool, +1 for paranoia */
extern char *genocide;			/* List of monsters to genocide */
extern char genocided[MU_BUF + 1];	/* List of monsters genocided, +1 for paranoia */
extern char lastcmd[MU_BUF + 1];	/* Copy of last command sent to Rogue, +1 for paranoia */
extern char lastname[NAMSIZ + 1];	/* Name of last potion/scroll/wand, +1 for paranoia */
extern char nextid;			/* Next object to identify */
extern char screen[R][C + 1];		/* Map of current rogue screen - characters drawn by rogue, +1 for paranoia */
extern char sumline[BIGBUF + 1];	/* Termination message for Rogomatic, +1 for paranoia */
extern char sumline2[BIGBUF + 1];	/* alternate sumline buffer, +1 for paranoia */
extern char ourkiller[MU_BUF + 1];	/* What was listed on the tombstone - How we died, +1 for paranoia */
extern char pending_call_letter;	/* If non-blank we have a call it to do - Pack object we know a name for */
extern char pending_call_name[NAMSIZ + 1];	/* Pack object name for letter, +1 for paranoia */
extern char versionstr[MU_BUF + 1];		/* Version of Rogue being used, +1 for paranoia */
extern char rgmdir[SM_BUF + 1];			/* rogomatic directory - may include UTC date and time sub-dir, +1 for paranoia */
extern char lock_path[SM_BUF + 1];		/* rogomatic lock file path, +1 for paranoia */
extern char roguename[MU_BUF + 1];	/* Name we are playing under, +1 for paranoia */
extern char *termination;		/* Latin verb for how we died */

/* global integers */
extern int aggravated;		/* True if we aggravated this level */
extern int agoalr;		/* Goal square to arch from (row) */
extern int agoalc;		/* Goal square to arch from (col) */
extern int ammo;		/* Number of missiles in pack */
extern bool arrowshot;		/* True if trap fired at us this round */
extern int atrow;		/* Current position of the rogue (@) (row) */
extern int atcol;		/* Current position of the rogue (@) (col) */
extern int atrow0;		/* Position at start of turn (row) */
extern int atcol0;		/* Position at start of turn (col) */
extern int attempt;		/* Number times we searched whole level */
extern bool badarrow;		/* True if we missed with this arrow */
extern bool beingheld;		/* True if being held by a fungus */
extern int beingstalked;	/* Invisible stalker strategies */
extern bool blinded;		/* True if blinded */
extern int blindir;		/* Last direction we moved when blind */
extern int cancelled;		/* Turns till use cancellation again */
extern bool cheat;		/* True ==> cheat to win */
extern bool checkrange;		/* True ==> check range */
extern bool chicken;		/* True ==> test run away code */
extern bool compression;	/* True ==> move multiple squares */
extern bool confused;		/* True if confused */
extern bool cosmic;		/* True if hallucinating */
extern int currentarmor;	/* Index of our armor */
extern int currentweapon;	/* Index of our weapon */
extern bool cursedarmor;	/* True if our armor is cursed */
extern bool cursedweapon;	/* True if we are wielding cursed weapon */
extern int darkdir;		/* Direction of arrow in dark room */
extern int darkturns;		/* Number of arrows left to fire in dark room */
extern int debugging;		/* Debugging options in effect */
extern int didreadmap;		/* Last magically mapped level */
extern int doorlist[40];	/* Holds row or column of new doors found on this level */
extern bool doublehasted;	/* True if double hasted (3.6 only) */
extern int droppedscare;	/* Number of scare mon. on this level */
extern bool diddrop;		/* True if we dropped anything on this spot */
extern bool emacs;		/* True ==> format output for Emacs */
extern bool exploredlevel;	/* True if we completely explored this level */
extern bool floating;		/* True if we are levitating */
extern bool foughtmonster;	/* True if we recently fought a monster */
extern bool foundarrowtrap;	/* Found arrow trap this level */
extern bool foundtrapdoor;	/* Found trap door this level */
extern int goalr;		/* Current goal square (row) */
extern int goalc;		/* Current goal square (col) */
extern int goodarrow;		/* Number of times we killed in one blow */
extern bool goodweapon;		/* Used for two-handed sword */
extern int gplusdam;		/* Our plus damage bonus from strength */
extern int gplushit;		/* Our plus to hit bonus from strength */
extern bool hasted;		/* True if hasted */
extern int head;		/* starting index of circular queue */
extern int tail;		/* ending index of circular queue */
extern int hitstokill;		/* Number of hits to kill last monster */
extern bool interrupted;	/* True if at commandtop from onintr() */
extern bool knowident;		/* True if found an identify scroll */
extern int larder;		/* Number of foods left */
extern int lastate;		/* Time we last ate */
extern int lastdamage;		/* Amount of last hit by a monster */
extern int lastdrop;		/* Last object we tried to drop */
extern int lastfoodlevel;	/* Last level we found food */
extern int lastmonster;		/* Last monster we tried to hit */
extern int lastobj;		/* What did we last try to use */
extern int lastwand;		/* Index of last wand */
extern int leftring;		/* Index of our left ring */
extern bool logdigested;	/* True if log file has been read by replay */
extern bool logging;		/* True if keeping record of game */
extern bool lyinginwait;	/* True if we waited for a monster */
extern int maxobj;		/* How much can we carry */
extern bool missedstairs;	/* True if we missed the stairs */
extern int morecount;		/* Number of messages since last command */
extern bool msgonscreen;	/* True if there is a rogomatic msg on the screen */
extern bool newarmor;		/* True if our armor status has changed */
extern int *newdoors;		/* pointer to a row or column of a new door found on this level */
extern bool newring;		/* True if our ring status has changed */
extern bool newweapon;		/* True if our weapon status has changed */
extern bool nohalf;		/* True if no halftime show */
extern bool noterm;		/* True if no human watching */
extern int objcount;		/* Number of objects */
extern int ourscore;		/* Our score when we died/quit */
extern bool playing;		/* True if still playing the game */
extern bool poorarrow;		/* True if arrow has missed */
extern bool protected;		/* True if we protected our armor */
extern int putonseeinv;		/* Turn when last put on see inv ring */
extern bool redhands;		/* True if our hands are red */
extern bool replaying;		/* True if replaying old game */
extern bool revvideo;		/* True if in rev. video mode */
extern int rightring;		/* Index of our right ring */
extern int rogpid;		/* Process id of rogue process */
extern int room[9];		/* Flags for each room */
extern int row;			/* Current cursor position (row) */
extern int col;			/* Current cursor position (col) */
extern int scrmap[R][C + 1];	/* attribute flags for squares, +1 for paranoia */
extern bool slowed;		/* True if we recently slowed a monster */
extern int stairrow;		/* Position of stairs on this level (row) */
extern int staircol;		/* Position of stairs on this level (col) */
extern int teleported;		/* Number of times teleported on this level */
extern bool terse;		/* True if in terse mode */
extern bool transparent;	/* True if in user command mode */
extern int trapr;		/* Location of arrow trap, this level (row) */
extern int trapc;		/* Location of arrow trap, this level (col) */
extern int urocnt;		/* Un-identified Rogue Object count */
extern int usesynch;		/* True when the inventory is correct - finished using something */
extern bool usingarrow;		/* True if wielding an arrow from a trap */
extern int version;		/* rogue version is an integer as set by getrougeversion() */
extern int wplusdam;		/* Our plus damage from weapon bonus */
extern int wplushit;		/* Our plus hit from weapon bonus */
extern int zone;		/* Current zone (0 to 8) */
extern int zonemap[9][9];	/* Connectivity map - Map of zones connections */

/* Status line variables */
extern int Level, MaxLevel, Gold, Hp, Hpmax, Str, Strmax, Ac, Exp, Explev;
extern char Ms[];		/* Msg 'X', 'Hungry', 'Weak', 'Fainting' */
extern int turns;		/* Est time in Rogue turns since start */

/* Geometry data */
extern int deltc[8], deltr[8];	/* Displacements for directions */
extern int deltrc[8];		/* ditto */
extern char keydir[8];		/* Directions for motion keys */
extern int movedir;		/* Which direction did we last move */
extern stuff translate[128];	/* what Rogue characters represent */

/* Time history */
extern timerec timespent[50];

/* Objects in pack */
extern invrec inven[MAXINV]; extern int invcount;

/* Stuff on this level */
extern stuffrec slist[MAXSTUFF + 1];
extern int slistlen;

/* Monster on this level */
extern monrec mlist[MAXMONST + 1];
extern int mlistlen;

extern char	killedmonster, targetmonster;

/* Door search variables */
extern bool	new_mark;
extern bool	new_findroom;
extern bool	new_search;
extern bool	new_stairs;
extern bool	new_arch;
extern char	timessearched[R][C + 1]; /* +1 for paranoia */
extern char	timestosearch;
extern int	searchstartr, searchstartc;
extern int	reusepsd;

/* Results of last makemove */
extern int	ontarget, targetrow, targetcol;

/* Monster attribute and Long term memory arrays */
extern attrec monatt[Z + 1];		/* Monster attributes */
extern lrnrec ltm;			/* Long term memory -- general */
extern ltmrec monhist[MAXMON + 1];	/* Long term memory -- creatures */
extern int nextmon;			/* Length of LTM */
extern int monindex[MAXMONST + 1];	/* Index into monhist array */

/* Genetic learning arrays */
extern int k_srch;		/* Propensity for searching squares */
extern int k_door;		/* Propensity for searching doors */
extern int k_rest;		/* Propensity for resting */
extern int k_arch;		/* Propensity for firing arrows */
extern int k_exper;		/* Level on which to experiment with items */
extern int k_run;		/* Propensity for retreating */
extern int k_wake;		/* Propensity for waking things up */
extern int k_food;		/* Propensity for hoarding food (rings) */


#endif
