/*
 * globals.h: Rog-O-Matic XIV (CMU) Thu Jan 31 18:12:50 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Global variables
 */

/* global files */
extern FILE *frogue,*trogue;	/* From Rogue, To Rogue */
extern FILE *fecho;		/* Rogomatic score file */
extern FILE *logfile;		/* Rogomatic score file */
extern FILE *realstdout;	/* Real stdout when in terse or emacs mode */
extern FILE *snapshot;		/* File for snapshot command */
FILE *wopen();			/* Open a file for world access */

/* global characters and strings */
extern char afterid;		/* Index of object after identify */
extern char dropid;		/* Next object to drop */
extern char wieldid;		/* Next item to wield */
extern char *genocide;		/* List of monsters to genocide */
extern char genocided[];	/* List of monsters genocided */
extern char lastcmd[];		/* Copy of last command sent to Rogue */
extern char lastname[];		/* Name of last potion/scroll/wand */
extern char nextid;		/* Next object to identify */
extern char ourkiller[];	/* What was listed on the tombstone */
extern char *parmstr;		/* Pointer to argument space */
extern char queue[];		/* stuff to be sent to Rogue */
extern char roguename[241];		/* Name we are playing under */
extern char screen[24][80];	/* characters drawn by Rogue */
extern char sumline[];		/* Summation line */
extern char *termination;	/* Latin verb for how we died */
extern char versionstr[];	/* Version of Rogue we are playing */

/* character and string functions */
extern int getlogtoken();
extern char *getname(), *itemstr();
extern char logchar(), *monname(), *realname(stuff, char *);

/* double precision floating point functions */
double prob(), mean(), stdev();	/* For stats.c */

/* global integers */
extern int aggravated;		/* True if we aggravated this level */
extern int agoalr,agoalc;	/* where we killed a monster */
extern int ammo;		/* Number of missiles in pack */
extern int arglen;		/* Length of argument space */
extern int arrowshot;		/* True if trap fired at us this round */
extern int atrow,atcol;		/* where is the '@'? (us) */
extern int atrow0,atcol0;	/* where was the '@' last time */
extern int attempt;		/* # times have we explored this level */
extern int badarrow;		/* True if we missed with this arrow */
extern int beingheld;		/* True if being held by a fungus */
extern int beingstalked;	/* True if an Invisible Stalker is around */
extern int blinded;		/* True if blinded */
extern int blindir;		/* Last direction we moved when blind */
extern int cancelled;		/* Turns till use cancellation again */
extern int cecho;		/* Last message type to logfile */
extern int cheat;		/* True ==> cheat to win */
extern int checkrange;		/* True ==> check range */
extern int chicken;		/* True ==> test run away code */
extern int compression;		/* True ==> move multiple squares */
extern int confused;		/* True if confused */
extern int cosmic;		/* True if hallucinating */
extern int currentarmor;	/* Index of our armor */
extern int currentweapon;	/* Index of our weapon */
extern int cursedarmor;		/* True if armor is cursed */
extern int cursedweapon;	/* True if weapon if cursed */
extern int darkdir;		/* Direction of arrow in dark room */
extern int darkturns;		/* # arrows left to fire in dark room */
extern int debugging;		/* Debugging options in effect */
extern int didfight;            /* Last command caused fighting */
extern int didreadmap;		/* Last magically mapped level */
extern int doorlist[], *newdoors; /* Holds r,c of new doors found */
extern int doublehasted;	/* True if double hasted (3.6 only) */
extern int droppedscare;	/* Number of scare mon. on this level */
extern int emacs;		/* True if in emacs mode */
extern int exploredlevel;	/* We completely explored this level */
extern int floating;		/* True if we are levitating */
extern int foughtmonster;	/* True if we recently fought a monster */
extern int foundarrowtrap;	/* Well, did we? */
extern int foundtrapdoor;	/* Well, did we? */
extern int goalr,goalc;		/* where are we trying to go */
extern int goodarrow;		/* Number of times we killed in one blow */
extern int goodweapon;		/* Used for two-handed sword */
extern int gplusdam;		/* Global damage bonus */
extern int gplushit;		/* Global hit bonus */
extern int hasted;		/* True if hasted */
extern int head,tail;		/* endpoints of circular queue */
extern int hitstokill;		/* Number of hits to kill last monster */
extern int interrupted;		/* True if at commandtop from onintr() */
extern int knowident;		/* Found an identify scroll? */
extern int larder;		/* Number of foods left */
extern int lastate;		/* Time we last ate */
extern int lastdamage;		/* Amount of last hit by a monster */
extern int lastdrop;		/* What did we last try to drop */
extern int lastfoodlevel;	/* Last food found */
extern int lastmonster;		/* Last monster we tried to hit */
extern int lastobj;		/* What did we last try to use */
extern int lastwand;		/* Index of last wand */
extern int leftring;		/* Index of our left ring */
extern int logdigested;		/* True if game log has been read by replay */
extern int logging;		/* True if logging game */
extern int lyinginwait;		/* Did we lie in wait last turn? */
extern int maxobj;		/* How much can we carry */
extern int missedstairs;	/* True if we missed the stairs */
extern int morecount;		/* Number of messages since last command */
extern int msgonscreen;		/* There is a rogomatic msg on the screen */
extern int newarmor;		/* True if our armor status has changed */
extern int newring;		/* True if our ring status has changed */
extern int newweapon;		/* True if our weapon status has changed */
extern int nohalf;		/* True if no halftime show */
extern int noterm;		/* True if no human watching */
extern int objcount;		/* Number of objects */
extern int ourscore;		/* Our score when we died/quit */
extern int playing;		/* True if still playing the game */
extern int poorarrow;		/* # Times we failed to kill in one blow */
extern int protected;		/* True if we protected our armor */
extern int putonseeinv;		/* Time when last put on see invisible ring */
extern int quitat;		/* Score we are trying to beat */
extern int redhands;		/* True if our hands are red */
extern int replaying;		/* True if replaying old game */
extern int revvideo;		/* True if in rev. video mode */
extern int rightring;		/* Index of our right ring */
extern int rogpid;		/* Process id of Rogue process */
extern int room[];		/* Flags for each room */
extern int row,col;		/* where is the cursor? */
extern int scrmap[24][80];	/* attribute flags for squares */
extern int slowed;		/* True if we recently slowed a monster */
extern int stairrow,staircol;	/* Where is the staircase */
extern int teleported;		/* times teleported on this level */
extern int terse;		/* True if in terse mode */
extern int transparent;		/* True ==> user mode */
extern int trapc;		/* Col of last trap */
extern int trapr;		/* Row of last trap */
extern int urocnt;		/* Un-identified Rogue Object count */
extern int usesynch;		/* Have we finished using something? */
extern int usingarrow;		/* True if wielding an arrow from a trap */
extern int version;		/* From types.h, set by getrougeversion */
extern int wplusdam;		/* Weapon damage bonus */
extern int wplushit;		/* Weapon hit bonus */
extern int zone;		/* Current zone (0 to 8) */
extern int zonemap[9][9];	/* Connectivity map */

/* Status line variables */
extern int Level,MaxLevel,Gold,Hp,Hpmax,Str,Strmax,Ac,Exp,Explev;
extern char Ms[];		/* Msg 'X', 'Hungry', 'Weak', 'Fainting' */
extern int turns;		/* Est time in Rogue turns since start */

/* Geometry data */
extern int deltc[], deltr[];	/* Displacements for directions */
extern int deltrc[];		/* ditto */
extern char keydir[];		/* Directions for motion keys */
extern int movedir;		/* Which direction did we last move */
extern stuff translate[];	/* what Rogue characters represent */

/* Time history */
extern timerec timespent[];

/* Objects in pack */
extern invrec inven[];	extern int invcount;

/* Stuff on this level */
extern stuffrec slist[]; extern int slistlen;

/* Monster on this level */
extern monrec mlist[];	extern int mlistlen;

extern char	killedmonster, targetmonster;

/* Door search variables */
extern int	new_mark, new_findroom, new_search, new_stairs, new_arch;
extern char	timessearched[24][80], timestosearch, attempttosearch;
extern int	searchstartr, searchstartc;
extern int	reusepsd;

/* Results of last makemove */
extern int	ontarget, targetrow, targetcol;

/* Monster attribute and Long term memory arrays */
extern attrec monatt[];		/* Monster attributes */
extern lrnrec ltm;		/* Long term memory -- general */
extern ltmrec monhist[];	/* Long term memory -- creatures */
extern ltmrec delhist[];	/* Long term memory -- changes this game */
extern int nextmon;		/* Length of LTM */
extern int monindex[];		/* Index into monhist array */

/* Genetic learning arrays */
extern int knob[];		/* Knobs */
extern int k_srch;		/* Propensity for searching squares */
extern int k_door;		/* Propensity for searching doors */
extern int k_rest;		/* Propensity for resting */
extern int k_arch;		/* Propensity for firing arrows */
extern int k_exper;		/* Level on which to experiment with items */
extern int k_run;		/* Propensity for retreating */
extern int k_wake;		/* Propensity for waking things up */
extern int k_food;		/* Propensity for hoarding food (rings) */

extern int stlmatch (char *, char *);
extern int willrust (int);
extern int armorclass (int);
extern int havenamed (stuff, char *);
extern int weaponclass (int);
extern int ringclass (int);
extern int havefood (int);
extern int hitbonus (int);
extern int damagebonus (int);
extern int bowclass (int);
extern int wearing (char *);
extern int havemult (stuff, char *, int);
extern int wielding (stuff);
extern int dwait(int, char *, ...);
extern int showcommand (char *);
extern int clearcommand ();
extern int wakemonster (int);
extern int sleepmonster ();
extern int doresetinv ();
extern int commandcount ();
extern int adjustpack (char *);
extern int bumpsearchcount ();
extern int send (char *, ...);
extern void sendcnow (int);
extern char commandarg (char *, int);
extern int deleteinv (int);
extern int removeinv (int);
extern int usemsg (char *, int);
extern int useobj (stuff, char *);
extern int setbonuses ();
extern int getoldcommand (char *);
extern int at (int, int);
extern int printsnap (FILE *);
extern int dosnapshot ();
extern int getscrpos (char *, int *,int *);
extern int summary (FILE *, int);
extern int dumpflags (int, int);
extern int dumpwalls ();
extern int dumpmonster ();
extern int dumpstuff ();
extern int quitrogue (char *, int, int);
extern int say (char *, ...);
extern int command(int tmode, char *f, ...);
extern int sendnow(char *f, ...);
extern int saynow(char *f, ...);
extern int dumpinv (FILE *);
extern int toggledebug ();
extern int promptforflags ();
extern int dumpdatabase ();
extern int markcycles ();
extern int terpmes ();
extern int redrawscreen ();
extern int reset_int ();
extern int deadrogue ();
extern void add_score (char *, char *, int);
extern int addprob (probability *, int);
extern int resetinv ();
extern int doresetinv ();
extern void toggleecho ();
extern int initstufflist ();
extern int addstat (statistic *, int);
extern void updateat ();
extern int printexplored ();
extern int mapinfer ();
extern int clearscreen ();
extern int waitforspace ();
extern int terpbot ();
extern int currentrectangle ();
extern int setnewgoal ();
extern int newlevel ();
extern int findmonster (char *);
extern int deletestuff(int, int);
extern void inferhall(int, int);
extern int updatepos (int, int, int);
extern int fexists (char *);
extern int quit (int, char *, ...);
extern int clearstat (statistic *);
extern int pickgenotype ();
extern int parsestat (char *, statistic *);
extern int writestat (FILE *, statistic *);
extern int randint (int);
extern int setnewgoal ();
extern int critical ();
extern int uncritical ();
extern int startreplay (FILE **, char *);
extern int closelog ();
extern int havebow (int, int);
extern int clearsendqueue ();
extern int endlesson ();
extern int getrogver ();
extern int restoreltm ();
extern int startlesson ();
extern int resend ();
extern int versiondep ();
extern int strategize ();
extern void getrogue (char *, int);
extern int charsavail ();
extern int givehelp ();
extern int dumpmonstertable ();
extern int pauserogue ();
extern int dumpmazedoor ();
extern int have (stuff);
extern int setpsd (int);
extern int havearmor (int, int, int);
extern int havering (int, int);
extern int haveweapon (int, int);
extern void positionreplay ();
extern void saveltm (int);
extern void evalknobs (int, int, int);
extern int setknobs (int *, int *, int *, int *);
extern int readgenes (char *);
extern int writegenes (char *);
extern int lock_file (char *, int);
extern int timehistory (FILE *, int);
extern int initpool (int, int);
extern int unlock_file (char *);
extern void mshit (char *);
extern void msmiss (char *);
extern int holdmonsters ();
extern int didhit ();
extern int didmiss ();
extern void curseditem ();
extern int wasmissed (char *);
extern int infer (stuff, char *);
extern int washit (char *);
extern int findstairs (int, int);
extern void nametrap (int, int);
extern int waitfor (char *);
extern int analyzeltm ();
extern int seemonster (char *);
extern int plusweapon ();
extern int rampage ();
extern int eat ();
extern int countgold (char *);
extern int readident (char *);
extern int unknown (stuff);
extern int mapcharacter (int, char *);
extern int monsternum (char *);
extern int killed (char *);
extern int smatch (char *, char *, char **);
extern int infername (stuff, int, char *, char *);
extern int getmonhist (char *, int);
extern int parsemsg (char *, char *);
extern int inventory (char *, char *);
extern int countpack ();
extern void clearpack (int);
extern void rollpackup (int);
extern int worth (int);
extern int used (stuff, char *);
extern int avoid ();
extern int expinit ();
extern int canbedoor (int, int);
extern int pinavoid ();
extern int nexttowall (int, int);
extern int markexplored (int, int);
extern int avoidmonsters ();
extern int darkroom ();
extern int markmissingrooms ();
extern int nextto (int, int, int);
extern int mazedoor (int, int);
extern int whichroom (int, int);
extern int isexplored (int, int);
extern int makemove (int, int (*)(), int (*)(), int);
extern int caddycorner (int, int, int, int, int);
extern int zigzagvalue (int, int, int, int *, int *, int *);
extern void parsemonster (char *);
extern int wakemonster (int);
extern int readltm ();
extern int parseprob (char *, probability *);
extern int addmonhist (char *);
extern int clearltm (ltmrec *);
extern int clearprob (probability *);
extern int writeprob (FILE *, probability *);
extern int isholder (char *);
extern int deletemonster (int, int);
extern int findmatch (FILE *f, char *s);
extern int unrest ();
extern int cancelmove (int);
extern int unmarkexplored (int, int);
extern int newmonsterlevel ();
extern int foundnew ();
extern int havewand (char *);
extern int reads (int);
extern int teleport ();
extern int addmonster (int, int, int, int);
extern int addstuff (int, int, int);
extern int point (int, int);
extern int connectdoors (int, int, int, int);
extern int filelength (char *);
extern int int_exit (void (*)());
extern int findmove (int, int (*)(), int (*)(), int);
extern int rmove (int, int, int);
extern int followmap (int);
extern int validatemap (int, int (*)(), int (*)());
extern int searchfrom (int, int, int (*)(), char [24][80], int *, int *);
extern int fmove (int);
extern int takeoff ();
extern int mmove (int, int);
extern int searchto (int, int, int (*)(), char [24][80], int *, int *);
extern int author ();
extern int replaylog (char *, char *);
extern int findscore (char *, char *);
extern int dumpscore (char *);
extern int fightmonster ();
extern int readscroll ();
extern int handlearmor ();
extern int tomonster ();
extern int shootindark ();
extern int readscroll ();
extern int handleweapon ();
extern int quaffpotion ();
extern int fightinvisible ();
extern int unpin ();
extern int haveuseless ();
extern int havemissile ();
extern int canrun ();
extern int fainting ();
extern int quitforhonors ();
extern int lightroom ();
extern int findring (char *);
extern int quaff (int);
extern int drop (int);
extern int wield (int);
extern int runaway ();
extern int backtodoor (int);
extern int throw (int,int);
extern int light ();
extern int dinnertime ();
extern int handlering ();
extern int display (char *);
extern int tostuff ();
extern int goupstairs (int);
extern int trywand ();
extern int gotowardsgoal ();
extern int dropjunk ();
extern int restup ();
extern int aftermelee ();
extern int exploreroom ();
extern int archery ();
extern int findarrow ();
extern int findroom ();
extern int godownstairs (int);
extern int battlestations ();
extern int wanttowake (int);
extern int replaycommand ();
extern int grope (int);
extern int pickupafter ();
extern int plunge ();
extern int gotowards (int, int, int);
extern int checkcango (int, int);
extern int doorexplore ();
extern int seeawakemonster (char *);
extern int puton (int);
extern int gotocorner ();
extern int hungry ();
extern int archmonster (int, int);
extern int move1 (int);
extern int markchokepts ();
extern int wear (int);
extern int know (stuff, char *);
extern int prepareident (int, int);
extern int findsafe ();
extern int waitaround ();
extern int downright (int *, int *);
extern int movetorest ();
extern int havearrow ();
extern int haveexplored (int);
extern int halftimeshow (int);
extern int waitaround ();
extern int removering (int);
extern int haveother (stuff, int);
extern int haveminus ();
extern int pickident (int);
extern int unidentified (stuff);
extern int useless (int);
extern int analyzepool (int);
extern FILE * openlog (char *);
extern int dropreply ();
extern char functionchar (char *);
extern int wieldreply ();
