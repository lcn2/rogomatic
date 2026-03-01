/*
 * setup.c: Rog-O-Matic XIV (CMU) Wed Jan 30 17:38:07 1985 - mlm
 * Copyright (C) 1985 by A. Appel, G. Jacobson, L. Hamey, and M. Mauldin
 *
 * This is the program which forks and execs the Rogue & the Player
 */

# include <stdio.h>
# include <signal.h>
# include <stdlib.h>
# include <fcntl.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include <string.h>

# include "types.h"
# include "globals.h"
# include "install.h"

# define READ    0
# define WRITE   1

/* Define the Rog-O-Matic pseudo-terminal (Concept Based) */

# define ROGUETERM "rg|rterm:am:bs:ce=^[^S:cl=^L:cm=^[a%+ %+ :co#80:li#24:so=^[D:se=^[d:pt:ta=^I:up=^[;:db:xn:"
# define ROGUETERMINFO "rg|rterm|Rog-O-Matic Pseudo Terminal,auto_right_margin,eat_newline_glitch,memory_below,columns#80," \
		       "init_tabs#8,lines#24,bell=^G,carriage_return=^M,clear_screen=^L,clr_eol=\\E^S," \
		       "cursor_address=\\Ea%p1%' '%+%c%p2%' '%+%c,cursor_down=\\E<,cursor_left=^H,cursor_up=\\E;," \
		       "enter_standout_mode=\\ED,exit_standout_mode=\\Ed,key_backspace=^H,key_down=^J,key_left=^H," \
		       "newline=^M^J,scroll_forward=^J,tab=^I,"

int   rfrogue, rtrogue;
extern char *getname(void);

int
main (int argc, char *argv[])
{ int   ptc[2], ctp[2];
  int   child, score = 0, oldgame = 0;
  int   cheat = 0, noterm = 1, echo = 0, nohalf = 0, replay = 0;
  int   emacs = 0, rf = 0, terse = 0, user = 0, quitat = 2147483647;
  char  *rfile = "", *rfilearg = "";
  char	options[MU_BUF + 1]; /* rogomatic options, +1 for paranoia */
  char  ropts[SM_BUF + 1]; /* rogue options, +1 for paranoia */
  char  roguename[MU_BUF + 1]; /* rogue player name, +1 for paranoia */

  /* zeroize arrays */
  memset (options, 0, sizeof(options)); /* paranoia */
  memset (ropts, 0, sizeof(ropts)); /* paranoia */
  memset (roguename, 0, sizeof(roguename)); /* paranoia */

  while (--argc > 0 && (*++argv)[0] == '-')
  { while (*++(*argv))
    { switch (**argv)
      { case 'c': cheat++;        break; /* Will use trap arrows! */
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

    if (rf)
    { if (--argc) rfilearg = *++argv;
      rf = 0;
    }
  }

  if (argc > 1)
  { printf ("Usage: rogomatic [-cefhprstuwE] or rogomatic <file>\n");
    exit (1);
  }

  /* Find which rogue to use */
  if (*rfilearg)
  { if (access (rfilearg, 1) == 0)	rfile = rfilearg;
    else				{ perror (rfilearg); exit (1); }
  }
  else if (access ("rogue", 1) == 0)	rfile = "rogue";
# ifdef NEWROGUE
  else if (access (NEWROGUE, 1) == 0)	rfile = NEWROGUE;
# endif
# ifdef ROGUE
  else if (access (ROGUE, 1) == 0)	rfile = ROGUE;
# endif
  else
  { perror ("rogue");
    exit (1);
  }

  if (!replay && !score) quitat = findscore (rfile, "Rog-O-Matic");

  snprintf (options, MU_BUF, "%d,%d,%d,%d,%d,%d,%d,%d",
            cheat, noterm, echo, nohalf, emacs, terse, user,quitat);
  snprintf (roguename, MU_BUF, "Rog-O-Matic %s for %s", RGMVER, getname ());
  snprintf (ropts, SM_BUF, "name=rogo-%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
            getname (), "fruit=apricot", "terse", "noflush", "noask",
            "jump", "step", "nopassgo", "inven=slow", "seefloor",
	    "tombstone", "file=/tmp/rogue.sav", "score=/tmp/rogue.scr",
	    "lock=/tmp/rogue.lck");

  if (score)  { dumpscore (argc==1 ? argv[0] : DEFVER); exit (0); }
  if (replay) { replaylog (argc==1 ? argv[0] : ROGUELOG, options); exit (0); }

  if ((pipe (ptc) < 0) || (pipe (ctp) < 0))
  { fprintf (stderr, "Cannot get pipes!\n");
    exit (1);
  }

  rtrogue = ptc[WRITE];
  rfrogue = ctp[READ];

  if ((child = fork ()) == 0)
  { close (0);
    dup (ptc[READ]);
    close (1);
    dup (ctp[WRITE]);
    close (ptc[WRITE]); /* Close parent's (player's) unused end of the pipes */
    close (ctp[READ]);

    putenv ("TERMINFO", ROGUETERMINFO);
    putenv ("TERM", "rg");
    putenv ("ROGUEOPTS", ropts);
    if (oldgame)  execl (rfile, rfile, "-r", NULL);
    if (argc)     execl (rfile, rfile, argv[0], NULL);
    execl (rfile, rfile, NULL);
    _exit (1);
  }

  else
  { /* Encode the open files into a two character string */

    char ft[3] = "aa"; ft[0] += rfrogue; ft[1] += rtrogue;
    char rp[MU_BUF + 1]; /* rogue pid, +1 for paranoia */

    /* zeroize arrays */
    memset (rp, 0, sizeof(rp)); /* paranoia */

    close (ptc[READ]); /* Close child's (rogue's) unused end of the pipes */
    close (ctp[WRITE]);

    /* Pass the process ID of the Rogue process as an ASCII string */
    snprintf (rp, MU_BUF, "%d", child);

    if (!author ()) nice (4);

    execl ("player", "player", ft, rp, options, roguename, NULL);
# ifdef PLAYER
    execl (PLAYER, "player", ft, rp, options, roguename, NULL);
# endif
    printf ("Rogomatic not available, 'player' binary missing.\n");
    kill (child, SIGKILL);
  }
}

/*
 * replaylog: Given a log file name and an options string, exec the player
 * process to replay the game.  No Rogue process is needed (since we are
 * replaying an old game), so the rfrogue and rtrogue file descriptors are
 * given the fake value 'Z'.
 */

int
replaylog (char *fname, char *options)
{ execl ("player", "player", "ZZ", "0", options, fname, NULL);
# ifdef PLAYER
  execl (PLAYER, "player", "ZZ", "0", options, fname, NULL);
# endif
  printf ("Replay not available, 'player' binary missing.\n");
  exit (1);
}

/*
 * author:
 *	See if a user is an author of the program
 */

int
author (void)
{
  switch (getuid())
  { case 1337:	/* Fuzzy */
    case 1313:	/* Guy */
    case 1241:	/* Andrew */
    case 345:	/* Leonard */
    case 342:	/* Gordon */
		return 1;
    default:	return 0;
  }
}
