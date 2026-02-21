/*
 * globals.h: Rog-O-Matic XIV (CMU) Thu Jan 31 18:12:50 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * Global variables
 */

#if !defined(INCLUDE_GLOBALS_H)
#define INCLUDE_GLOBALS_H


/* global files */
extern FILE *frogue,*trogue;	/* From Rogue, To Rogue */
extern FILE *fecho;		/* Rogomatic score file */
extern FILE *logfile;		/* Rogomatic score file */
extern FILE *realstdout;	/* Real stdout when in terse or emacs mode */
extern FILE *snapshot;		/* File for snapshot command */
FILE *wopen(char *, char *);			/* Open a file for world access */

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
extern int getlogtoken(void);
extern char *getname(void), *itemstr(int);
extern char logchar(int), *monname(char), *realname(stuff, char *);

/* double precision floating point functions */
double prob(probability *), mean(statistic *), stdev(statistic *);	/* For stats.c */

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
extern int interrupted;		/* True if at command from onintr() */
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
extern void showcommand (char *);
extern void clearcommand (void);
extern void wakemonster (int);
extern void sleepmonster (void);
extern int commandcount (char *);
extern void adjustpack (char *);
extern void bumpsearchcount (void);
extern void my_send (char *, ...);
extern void sendcnow (int);
extern char commandarg (char *, int);
extern void deleteinv (int);
extern void removeinv (int);
extern void usemsg (char *, int);
extern void useobj (stuff, char *);
extern void setbonuses (void);
extern void getoldcommand (char *);
extern void at (int, int);
extern void printsnap (FILE *);
extern void dosnapshot (void);
extern int getscrpos (char *, int *,int *);
extern void summary (FILE *, int);
extern void dumpflags (int, int);
extern void dumpwalls (void);
extern void dumpmonster (void);
extern void dumpstuff (void);
extern void quitrogue (char *, int, int);
extern void say (char *, ...);
extern void command(int tmode, char *f, ...);
extern void sendnow(char *f, ...);
extern void saynow(char *f, ...);
extern void dumpinv (FILE *);
extern void toggledebug (void);
extern void promptforflags (void);
extern void dumpdatabase (void);
extern int markcycles (int);
extern void terpmes (void);
extern void redrawscreen (void);
extern int reset_int (void);
extern void deadrogue (void);
extern void add_score (char *, char *, int);
extern void addprob (probability *, int);
extern void resetinv (void);
extern void doresetinv (void);
extern void toggleecho (void);
extern void initstufflist (void);
extern void addstat (statistic *, int);
extern void updateat (void);
extern void printexplored (void);
extern void mapinfer (void);
extern void clearscreen (void);
extern void waitforspace (void);
extern void terpbot (void);
extern void currentrectangle (void);
extern void clearcurrect (void);
extern void setnewgoal (void);
extern void newlevel (void);
extern int findmonster (char *);
extern void deletestuff(int, int);
extern void inferhall(int, int);
extern void updatepos (int, int, int);
extern int fexists (char *);
extern int quit (int, char *, ...);
extern void clearstat (statistic *);
extern int pickgenotype (void);
extern void parsestat (char *, statistic *);
extern void writestat (FILE *, statistic *);
extern int randint (int);
extern void my_srand (int);
extern int my_rand (void);
extern int critical (void);
extern int uncritical (void);
extern void startreplay (FILE **, char *);
extern void closelog (void);
extern int havebow (int, int);
extern void clearsendqueue (void);
extern void endlesson (void);
extern void getrogver (void);
extern void restoreltm (void);
extern void startlesson (void);
extern int resend (void);
extern void versiondep (void);
extern int strategize (void);
extern void getrogue (char *, int);
extern int charsavail (void);
extern void givehelp (void);
extern void dumpmonstertable (void);
extern void pauserogue (void);
extern void dumpmazedoor (void);
extern int have (stuff);
extern int setpsd (int);
extern int havearmor (int, int, int);
extern int havering (int, int);
extern int haveweapon (int, int);
extern void positionreplay (void);
extern void saveltm (int);
extern void evalknobs (int, int, int);
extern void setknobs (int *, int *, int *, int *);
extern int readgenes (char *);
extern void writegenes (char *);
extern int lock_file (char *, int);
extern void timehistory (FILE *, int);
extern void initpool (int, int);
extern int unlock_file (char *);
extern void mshit (char *);
extern void msmiss (char *);
extern void holdmonsters (void);
extern void didhit (void);
extern void didmiss (void);
extern void curseditem (void);
extern void wasmissed (char *);
extern void infer (stuff, char *);
extern void washit (char *);
extern void findstairs (int, int);
extern void nametrap (int, int);
extern void waitfor (char *);
extern void analyzeltm (void);
extern int seemonster (char *);
extern void plusweapon (void);
extern void rampage (void);
extern int eat (void);
extern void countgold (char *);
extern void readident (char *);
extern int unknown (stuff);
extern void mapcharacter (int, char *);
extern int monsternum (char *);
extern void killed (char *);
extern int smatch (char *, char *, char **);
extern void infername (stuff, int, char *, char *);
extern int getmonhist (char *, int);
extern void parsemsg (char *, char *);
extern int inventory (char *, char *);
extern void countpack (void);
extern void clearpack (int);
extern void rollpackup (int);
extern int worth (int);
extern int used (stuff, char *);
extern int avoid (void);
extern int expinit (void);
extern int canbedoor (int, int);
extern void pinavoid (void);
extern int nexttowall (int, int);
extern void markexplored (int, int);
extern void avoidmonsters (void);
extern int darkroom (void);
extern void markmissingrooms (void);
extern int nextto (int, int, int);
extern int mazedoor (int, int);
extern int whichroom (int, int);
extern int isexplored (int, int);
extern int makemove (int, int (*)(void), int (*)(int, int, int, int *, int *, int *), int);
extern void caddycorner (int, int, int, int, int);
extern int zigzagvalue (int, int, int, int *, int *, int *);
extern void parsemonster (char *);
extern void wakemonster (int);
extern void readltm (void);
extern void parseprob (char *, probability *);
extern int addmonhist (char *);
extern void clearltm (ltmrec *);
extern void clearprob (probability *);
extern void writeprob (FILE *, probability *);
extern int isholder (char *);
extern void deletemonster (int, int);
extern int findmatch (FILE *f, char *s);
extern void unrest (void);
extern void cancelmove (int);
extern void unmarkexplored (int, int);
extern void newmonsterlevel (void);
extern void foundnew (void);
extern int havewand (char *);
extern int reads (int);
extern void teleport (void);
extern void addmonster (int, int, int, int);
extern void addstuff (int, int, int);
extern int point (int, int);
extern void connectdoors (int, int, int, int);
extern int filelength (char *);
extern int int_exit (void (*)(int));
extern int findmove (int, int (*)(void), int (*)(int, int, int, int *, int *, int *), int);
extern void rmove (int, int, int);
extern int followmap (int);
extern int validatemap (int, int (*)(void), int (*)(int, int, int, int *, int *, int *));
extern int searchfrom (int, int, int (*)(int, int, int, int *, int *, int *), char [24][80], int *, int *);
extern void fmove (int);
extern int takeoff (void);
extern void mmove (int, int);
extern int searchto (int, int, int (*)(int, int, int, int *, int *, int *), char [24][80], int *, int *);
extern int author (void);
extern int replaylog (char *, char *);
extern int findscore (char *, char *);
extern int dumpscore (char *);
extern int fightmonster (void);
extern int readscroll (void);
extern int handlearmor (void);
extern int tomonster (void);
extern int shootindark (void);
extern int readscroll (void);
extern int handleweapon (void);
extern int quaffpotion (void);
extern int fightinvisible (void);
extern int unpin (void);
extern int haveuseless (void);
extern int havemissile (void);
extern int canrun (void);
extern int fainting (void);
extern int quitforhonors (void);
extern int lightroom (void);
extern int findring (char *);
extern int quaff (int);
extern int drop (int);
extern int wield (int);
extern int runaway (void);
extern int backtodoor (int);
extern int throw (int, int);
extern int light (void);
extern int dinnertime (void);
extern int handlering (void);
extern void display (char *);
extern int tostuff (void);
extern int goupstairs (int);
extern int trywand (void);
extern int gotowardsgoal (void);
extern int dropjunk (void);
extern int restup (void);
extern int aftermelee (void);
extern int exploreroom (void);
extern int archery (void);
extern int findarrow (void);
extern int findroom (void);
extern int godownstairs (int);
extern int battlestations (int, char *, int, int, int, int, int, int);
extern int wanttowake (char);
extern int replaycommand (void);
extern int grope (int);
extern int pickupafter (void);
extern int plunge (void);
extern int gotowards (int, int, int);
extern int checkcango (int, int);
extern int doorexplore (void);
extern int seeawakemonster (char *);
extern int puton (int);
extern int gotocorner (void);
extern int hungry (void);
extern int archmonster (int, int);
extern void move1 (int);
extern void markchokepts (void);
extern int wear (int);
extern int know (stuff, char *);
extern int prepareident (int, int);
extern int findsafe (void);
extern int waitaround (void);
extern int downright (int *, int *);
extern int movetorest (void);
extern int havearrow (void);
extern int haveexplored (int);
extern void halftimeshow (int);
extern int waitaround (void);
extern int removering (int);
extern int haveother (stuff, int);
extern int haveminus (void);
extern int pickident (int);
extern int unidentified (stuff);
extern int useless (int);
extern void analyzepool (int);
extern FILE *openlog (char *);
extern void dropreply (void);
extern char functionchar (char *);
extern void wieldreply (void);
extern int runinit (void);
extern int runvalue (int, int, int, int *, int *, int *);
extern int expruninit (void);
extern int exprunvalue (int, int, int, int *, int *, int *);
extern int unpininit (void);
extern int expunpininit (void);
extern int expunpinvalue (int, int, int, int *, int *, int *);
extern int rundoorinit (void);
extern int rundoorvalue (int, int, int, int *, int *, int *);
extern void intrupscore (int);
extern int secretinit (void);
extern int secretvalue (int, int, int, int *, int *, int *);
extern int expvalue (int, int, int, int *, int *, int *);
extern int roominit (void);
extern int archeryinit (void);
extern int archeryvalue (int, int, int, int *, int *, int *);
extern int restinit (void);
extern int restvalue (int, int, int, int *, int *, int *);
extern int pending (void);
extern int getroguetoken (void);
extern char *statusline (void);
extern int genericinit (void);
extern int downvalue (int, int, int, int *, int *, int *);

#endif
