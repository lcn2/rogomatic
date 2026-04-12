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
 * io.c:
 *
 * This file contains all of the functions which deal with the real world.
 */

# include <stdlib.h>
# include <unistd.h>
# include <ctype.h>
# include <string.h>
# include <sys/ioctl.h>
# include <time.h>
# include <sys/wait.h>
# include <errno.h>
# include <signal.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "modern_curses.h"
# include "types.h"
# include "globals.h"
# include "install.h"
# include "termtokens.h"
# include "getroguetoken.h"

# define ROGUE_SECONDS (3)  /* seconds to wait for rogue to exit */

# define READ	0

/*
 * Charonscreen returns the current character on the screen (using
 * curses(3)).  This macro is based on the winch(win) macro.
 */
# define charonscreen(Y,X)	(A_CHARTEXT & mvwinch (stdscr, Y, X))

/* static declarations */
static char *month[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char screen00 = ' ';

/* Constants */

# define SENDQ 256

/* potion quaffing state */
enum p_state {
  p_unset = 0,	    /* no multi-message potion quaffing in progress */
  p_what = 1,	    /* "Quaff what? " seen */
  p_list = 2,	    /* "(* for list): " seen */
  p_more = 3,	    /* "--More--" seen */
  p_call = 4,	    /* "Call it: " seen */
  p_quest = 5,	    /* sent "?" reply to set proper name */
};
static enum p_state potion_state = p_unset;

/* scroll reading state */
enum s_state {
  s_unset = 0,	    /* no multi-message scroll reading in progress */
  s_what = 1,	    /* "Read what? " seen */
  s_list = 2,	    /* "(* for list): " seen */
  s_more = 3,	    /* "--More--" seen */
  s_call = 4,	    /* "Call it: " seen */
  s_quest = 5,	    /* sent "?" reply to set proper name */
};
static enum s_state scroll_state = s_unset;

/* The command queue */

static char  queue[SENDQ];             /* stuff to be sent to Rogue */

static int s_row1 = -100;  /* start scroll regions way out of bounds */
static int s_row2 = 100;   /* start scroll regions way out of bounds */

static void scrollup (void);
static void scrolldown (void);
static void printscreen (void);
static void sendcnow (char c);
static int pending (void);
static void deadrogue (void);
static void waitforspace (void);
static char **helpline;
static void putn (char c, FILE *f, int n);

static void waitforspace (void);
static void sendcnow (char c);

static void
scrollup (void)
{
  int r;
  int c;

  newdoors = doorlist;

  for (r = s_row1; r < s_row2; r++) {
    for (c = 0; c < C; c++) {
      if (valrc (r, c) && valrc (r+1, c)) {
	screen[r][c] = screen[r+1][c];
	updatepos (screen[r][c], r, c);
      }
    }
  }

  for (c = 0; c < C; c++) {
    if (valrc (s_row2, c)) {
      screen[s_row2][c] = ' ';
      updatepos (screen[s_row2][c], s_row2, c);
    }
  }
}

static void
scrolldown (void)
{
  int r;
  int c;

  for (r = s_row2; r > s_row1; r--) {
    for (c = 0; c < C; c++) {
      if (valrc (r, c) && valrc (r-1, c)) {
	screen[r][c] = screen[r-1][c];
	updatepos (screen[r][c], r, c);
      }
    }
  }

  for (c = 0; c < C; c++) {
    if (valrc (s_row1, c)) {
      screen[s_row1][c] = ' ';
      updatepos (screen[s_row1][c], s_row1, c);
    }
  }
}

static void
printscreen (void)
{
  int i, j;

  /* firewall */
  if (!valrc (row, col)) {
    return;
  }

  debuglog ("-- cursor  [%2d, %2d] [%c] [%3d] -------------------------------------------------\n", row, col, screen[row][col], screen[row][col]);
  debuglog ("             1111111111222222222233333333334444444444555555555566666666667777777777\n");
  debuglog ("   01234567890123456789012345678901234567890123456789012345678901234567890123456789\n");

  for (i=0; i < R; ++i) {
    debuglog ("%02d", i);

    if (i >= s_row1 && i <= s_row2) {
      debuglog ("*");
    }
    else {
      debuglog (" ");
    }

    for (j = 0; j < C; ++j) {
      if (i == row && j == col)
        debuglog ("_");
      else if (valrc (i,j)) {
        debuglog ("%c", screen[i][j]);
      }
    }

    debuglog ("\n");
  }

  debuglog ("--------------------------------------------------------------------------------\n");
}

static const char *
p_state_name (enum p_state state)
{
  switch (state) {
    case p_unset:
      return "p_unset";
    case p_what:
      return "p_what";
    case p_list:
      return "p_list";
    case p_more:
      return "p_more";
    case p_call:
      return "p_call";
    case p_quest:
      return "p_quest";
    default:
      break;
  }
  return "p_unknown";
}

static const char *
s_state_name (enum s_state state)
{
  switch (state) {
    case s_unset:
      return "s_unset";
    case s_what:
      return "s_what";
    case s_list:
      return "s_list";
    case s_more:
      return "s_more";
    case s_call:
      return "s_call";
    case s_quest:
      return "s_quest";
    default:
      break;
  }
  return "s_unknown";
}

/*
 * Getrogue: Sensory interface.
 *
 * Handle grungy low level terminal I/O. Getrogue reads tokens from the
 * Rogue process and interprets them, making the screen array an image of
 * the rogue level. Getrogue returns when the string waitstr has been read
 * and either the cursor is on the Rogue '@' or some other condition
 * implies that we have synchronized with Rogue.
 */

/* waitstr - String to synchronize with */
/*
 * onat - 0 ==> Wait for waitstr,
 *	  1 ==> Cursor on @ sufficient,
 *	  2 ==> [1] + send ';' when ever we eat a --More-- message
 */
void
getrogue (char *waitstr, int onat)
{
  char  ch;				/* rogue output character, or screen reading package token */
  char *quaff_what = "Quaff what? ";	/* FSM to check for "Quaff what? " */
  char *read_what = "Read what? ";	/* FSM to check for "Read what? " */
  char *call_it = "Call it: ";		/* FSM to check for "Call it: " */
  char *more = "--More--";		/* FSM to check for "--More--' */
  char *for_list = "(* for list): ";	/* FSM to check for "(* for list): " prompt */
  char *tombstone_grass = ")______";	/* FSM to check for ")______" (tombstone grass) */
  char *wait_msg = waitstr;		/* FSM to check for the wait msg */

  bool botprinted = false;
  int wasmapped = didreadmap;
  int *doors;
  static bool moved = false;
  int r;
  int c;
  int i, j;

  /* firewall */
  if (!valrc (row, col)) {
    return;
  }

  newdoors = doorlist;			/* no new doors found yet */
  atrow0 = atrow;			/* Save our current position */
  atcol0 = atcol;			/* Save our current position */

  if (moved) {				/* If we moved last time, put any */
    sleepmonster ();			/* Old monsters to sleep */
    moved = false;
  }

  /* debugging info */
  if debug(D_MESSAGE) {
    at (28,0);
    clrtoeol ();
    at (27,0);
    clrtoeol ();
    printw("getrogue: waitstr ->%s<-  onat %d.",
           waitstr, onat);
    at (row, col);
    refresh ();
  }

  /* While we have not reached the end of the Rogue input, read */
  /* characters from Rogue and figure out what they mean.       */
  while ((*wait_msg) ||
         ((!hasted || version != RV36A) &&
	  onat && screen[row][col] != '@')) {

    /*
     * get next rogue output character, or screen reading package token
     */
    ch = getroguetoken ();
    if (debug(D_MESSAGE)) {
      at (28,col);
      printw ("%s", unctrl(ch));
      at (row, col);
      refresh ();
    }

    /*
     * Tokens used by the screen reading package are NOT processed
     * for matching against various messages.  This is in case
     * the ncurses system generates such token, such as a clear
     * screen, or such as a cursor movement while rogue is
     * printing a message.
     */
    if (!is_token (ch)) {

      /*
       * If message ends in "Quaff what? ", note it
       */
      if (ch == *quaff_what) {
	++quaff_what;
	if (*quaff_what == '\0') {

	  /*
	   * force the scroll reading state to be unset
	   */
	  if (scroll_state != s_unset) {
	      debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_unset saw: \"Quaff what? \"\n",
			__func__, __FILE__, __LINE__, s_state_name(scroll_state));
	      scroll_state = s_unset;
	  }

	  /*
	   * start the potion quaffing state
	   */
	  debuglog ("%s: file: %s line: %d potion_state: %s ==> p_what saw: \"Quaff what? \"\n",
		    __func__, __FILE__, __LINE__, p_state_name(potion_state));
	  potion_state = p_what;
	}
      } else {
	quaff_what = "Quaff what? ";
      }

      /*
       * If message ends in "Read what? ", note it
       */
      if (ch == *read_what) {
	++read_what;
	if (*read_what == '\0') {

	  /*
	   * force the potion quaffing state to be unset
	   */
	  if (potion_state != p_unset) {
	      debuglog ("%s: file: %s line: %d potion_state: %s ==> p_unset saw: \"Read what? \"\n",
			__func__, __FILE__, __LINE__, p_state_name(potion_state));
	      potion_state = p_unset;
	  }

	  /*
	   * start the scroll reading state
	   */
	  debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_what saw: \"Read what? \"\n",
		    __func__, __FILE__, __LINE__, s_state_name(scroll_state));
	  scroll_state = s_what;
	}
      } else {
	read_what = "Read what? ";
      }

      /*
       * If message ends in "(* for list): ", call terpmes()
       */
      if (ch == *for_list) {
	++for_list;
	if (*for_list == '\0') {

	  /*
	   * advance the potion quaffing state if it was p_what
	   */
	  if (potion_state == p_what) {

	    /*
	     * force the scroll reading state to be unset
	     */
	    if (scroll_state != s_unset) {
              debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_unset saw: \"(* for list) \"\n",
                        __func__, __FILE__, __LINE__, s_state_name(scroll_state));
              scroll_state = s_unset;
	    }

	    /*
	     * advance potion quaff state
	     */
	    potion_state = p_list;
	    debuglog ("%s: file: %s line: %d potion_state: p_what ==> %s saw: \"(* for list) \"\n",
		      __func__, __FILE__, __LINE__, p_state_name(potion_state));

	  /*
	   * advance the scroll reading state if it was s_what
	   */
	  } else if (scroll_state == s_what) {

	    /*
	     * force the potion quaffing state to be unset
	     */
	    if (potion_state != p_unset) {
              debuglog ("%s: file: %s line: %d potion_state: %s ==> p_unset saw: \"(* for list) \"\n",
                        __func__, __FILE__, __LINE__, p_state_name(potion_state));
              potion_state = p_unset;
	    }

	    /*
	     * advance scroll reading state
	     */
	    scroll_state = s_list;
	    debuglog ("%s: file: %s line: %d scroll_state: s_what ==> %s saw: \"(* for list) \"\n",
		      __func__, __FILE__, __LINE__, s_state_name(scroll_state));

	  /*
	   * otherwise force the scroll reading, and force the potion quaffing state to be unset
	   */
	  } else {

	    /*
	     * force the scroll reading state to be unset
	     */
	    if (scroll_state != s_unset) {
              debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_unset saw: \"(* for list) \"\n",
                        __func__, __FILE__, __LINE__, s_state_name(scroll_state));
              scroll_state = s_unset;
	    }

	    /*
	     * force the potion quaffing state to be unset
	     */
	    if (potion_state != p_unset) {
              debuglog ("%s: file: %s line: %d potion_state: %s ==> p_unset saw: \"(* for list) \"\n",
                        __func__, __FILE__, __LINE__, p_state_name(potion_state));
              potion_state = p_unset;
	    }
	  }

	  /*
	   * parse top line message from rogue
	   */
	  terpmes ();
	}
      } else {
	for_list = "(* for list): ";
      }

      /*
       * If the message has a "--More--", strip it off and call terpmes()
       */
      if (ch == *more) {
	++more;
	if (*more == '\0') {

	  /*
	   * advance the potion quaffing state if it was p_list
	   */
	  if (potion_state == p_list) {

	    /*
	     * force the scroll reading state to be unset
	     */
	    if (scroll_state != s_unset) {
              debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_unset saw: \"--More--\"\n",
                        __func__, __FILE__, __LINE__, s_state_name(scroll_state));
              scroll_state = s_unset;
	    }

	    /*
	     * advance potion quaff state
	     */
	    potion_state = p_more;
	    debuglog ("%s: file: %s line: %d potion_state: p_list ==> %s saw: \"--More--\"\n",
		      __func__, __FILE__, __LINE__, p_state_name(potion_state));

	  /*
	   * advance the scroll reading state if it was s_list
	   */
	  } else if (scroll_state == s_list) {

	    /*
	     * force the potion quaffing state to be unset
	     */
	    if (potion_state != p_unset) {
              debuglog ("%s: file: %s line: %d potion_state: %s ==> p_unset saw: \"--More--\"\n",
                        __func__, __FILE__, __LINE__, p_state_name(potion_state));
              potion_state = p_unset;
	    }

	    /*
	     * advance scroll reading state
	     */
	    scroll_state = s_more;
	    debuglog ("%s: file: %s line: %d scroll_state: s_list ==> %s saw: \"--More--\"\n",
		      __func__, __FILE__, __LINE__, s_state_name(scroll_state));

	  /*
	   * otherwise force the scroll reading, and force the potion quaffing state to be unset
	   */
	  } else {

	    /*
	     * force the scroll reading state to be unset
	     */
	    if (scroll_state != s_unset) {
              debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_unset saw: \"--More--\"\n",
                        __func__, __FILE__, __LINE__, s_state_name(scroll_state));
              scroll_state = s_unset;
	    }

	    /*
	     * force the potion quaffing state to be unset
	     */
	    if (potion_state != p_unset) {
              debuglog ("%s: file: %s line: %d potion_state: %s ==> p_unset saw: \"--More--\"\n",
                        __func__, __FILE__, __LINE__, p_state_name(potion_state));
              potion_state = p_unset;
	    }
	  }

	  /* More than 50 messages since last command ==> start logging */
	  ++morecount;
	  if (morecount > 50 && !logging) {
	    toggleecho ();
	    dwait (D_WARNING, __func__, "Started logging --More-- loop");
	  }

	  /* Send a space (and possibly a semicolon) to clear the message */
	  if (onat == 2) {
	    if (version >= RV54B && potion_state == p_more) {
	      sendnow (" ");
	    } else {
	      sendnow (" ;");
	    }
	  } else {
	    sendnow (" ");
	  }

	  /* Clear the "--More--" of the end of the message */
	  for (i = col - 7; i < col; ++i) {
	    if (valrc (0,i)) {
	      screen[0][i] = ' ';
	    }
	  }

	  terpmes ();			/* Interpret the message */
	}
      } else {
	more = "--More--";
      }

      /*
       * If the message is "Call it: ", clear or process the call it the request
       */
      if (ch == *call_it) {
	++call_it;
	if (*call_it == '\0') {

	  /*
	   * advance the potion quaffing state if it was p_more
	   */
	  if (potion_state == p_list || potion_state == p_more) {

	    /*
	     * force the scroll reading state to be unset
	     */
	    if (scroll_state != s_unset) {
              debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_unset saw: \"Call it: \"\n",
                        __func__, __FILE__, __LINE__, s_state_name(scroll_state));
              scroll_state = s_unset;
	    }

	    /*
	     * advance potion quaff state
	     */
	    if (potion_state == p_list) {
	      potion_state = p_call;
	      debuglog ("%s: file: %s line: %d potion_state: p_list ==> %s saw: \"Call it: \"\n",
			__func__, __FILE__, __LINE__, p_state_name(potion_state));
	    } else if (potion_state == p_more) {
	      potion_state = p_call;
	      debuglog ("%s: file: %s line: %d potion_state: p_more ==> %s saw: \"Call it: \"\n",
			__func__, __FILE__, __LINE__, p_state_name(potion_state));
	    }

	  /*
	   * advance the scroll reading state if it was s_more
	   */
	  } else if (scroll_state == s_list || scroll_state == s_more) {

	    /*
	     * force the potion quaffing state to be unset
	     */
	    if (potion_state != p_unset) {
              debuglog ("%s: file: %s line: %d potion_state: %s ==> p_unset saw: \"Call it: \"\n",
                        __func__, __FILE__, __LINE__, p_state_name(potion_state));
              potion_state = p_unset;
	    }

	    /*
	     * advance scroll reading state
	     */
	    if (scroll_state == s_list) {
	      scroll_state = s_call;
	      debuglog ("%s: file: %s line: %d scroll_state: s_list ==> %s saw: \"Call it: \"\n",
			__func__, __FILE__, __LINE__, s_state_name(scroll_state));
	    } else if (scroll_state == s_more) {
	      scroll_state = s_call;
	      debuglog ("%s: file: %s line: %d scroll_state: s_more ==> %s saw: \"Call it: \"\n",
			__func__, __FILE__, __LINE__, s_state_name(scroll_state));
	    }

	  /*
	   * otherwise force the scroll reading, and force the potion quaffing state to be unset
	   */
	  } else {

	    /*
	     * force the scroll reading state to be unset
	     */
	    if (scroll_state != s_unset) {
              debuglog ("%s: file: %s line: %d scroll_state: %s ==> s_unset saw: \"Call it: \"\n",
                        __func__, __FILE__, __LINE__, s_state_name(scroll_state));
              scroll_state = s_unset;
	    }

	    /*
	     * force the potion quaffing state to be unset
	     */
	    if (potion_state != p_unset) {
              debuglog ("%s: file: %s line: %d potion_state: %s ==> p_unset saw: \"Call it: \"\n",
                        __func__, __FILE__, __LINE__, p_state_name(potion_state));
              potion_state = p_unset;
	    }
	  }

	  /*
	   * process a "Call it: " message
	   */
	  if (onat == 2) {

	    /*
	     * advance the potion quaffing state
	     */
	    if (version >= RV54B) {

	      if (potion_state == p_call) {

		/*
		 * advance potion quaffing state to p_quest
		 */
		potion_state = p_quest;
		debuglog ("%s: file: %s line: %d potion_state: p_call ==> %s saw: \"Call it: \"\n",
			  __func__, __FILE__, __LINE__, p_state_name(potion_state));

		/* since rogue 5.4.5, calling something ? sets the proper name be set automatically */
		debuglog ("%s: file: %s line: %d sending: \"?{nl};\"\n", __func__, __FILE__, __LINE__);
		sendnow ("?\n;");

	      /*
	       * advance the scroll reading state
	       */
	      } else if (scroll_state == s_call) {

		/*
		 * advance scroll reading state to s_quest
		 */
		scroll_state = s_quest;
		debuglog ("%s: file: %s line: %d scroll_state: s_call ==> %s saw: \"Call it: \"\n",
			  __func__, __FILE__, __LINE__, s_state_name(scroll_state));

		/* since rogue 5.4.5, calling something ? sets the proper name be set automatically */
		debuglog ("%s: file: %s line: %d sending: \"?{nl};\"\n", __func__, __FILE__, __LINE__);
		sendnow ("?\n;");
	      }
	    }

	    /* Send an escape and semicolon to clear the message */
	    debuglog ("%s: file: %s line: %d sending: \"ESC;\"\n", __func__, __FILE__, __LINE__);
	    sendnow ("%c;", ESC);

	  } else {

	      /* Send an escape to clear the message */
	      debuglog ("%s: file: %s line: %d sending: \"ESC\"\n", __func__, __FILE__, __LINE__);
	      sendnow ("%c", ESC);
	  }
	}
      } else {
	call_it = "Call it: ";
      }

      /*
       * Rogomatic now keys off of the grass under the Tombstone to
       * detect that it has been killed. This was done because the
       * "Press return" prompt only happens if there is a score file
       * Available on that system. Hopefully the grass is the same
       * in all versions of Rogue!
       */
      if (ch == *tombstone_grass) {
	++tombstone_grass;
	if (*tombstone_grass == '\0') {
	  debuglog ("%s: file: %s line: %d saw: \")_______\"\n", __func__, __FILE__, __LINE__);
	  addch (ch);
	  debuglog ("%s: file: %s line: %d calling: deadrogue()\n", __func__, __FILE__, __LINE__);
	  deadrogue ();
	  return;
	}
      } else {
	tombstone_grass = ")_______";
      }

      /*
       * Check to see whether we have read the synchronization string
       */
      if (*wait_msg) {
	if (ch == *wait_msg) {
	  wait_msg++;
	} else {
	  wait_msg = waitstr;
	}
      }
    }

    /* Now figure out what the token means */
    switch (ch) {
      case BS_TOK:
        col--;
        debuglog ("BS_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case CB_TOK:
        for (i =0; i < col; i++) {
          updatepos (' ', row, i);
          screen[row][i] = ' ';
        }

        debuglog ("CB_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case CE_TOK:
        if (row && row < R-1)
          for (i = col; i < C; i++) {
            updatepos (' ', row, i);
            screen[row][i] = ' ';
          }
        else
          for (i = col; i < C; i++)
            screen[row][i] = ' ';

        if (row) {
	  at (row, col);
	  clrtoeol ();
	} else if (col == 0) {
	  screen00 = ' ';
	}

        debuglog ("CE_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case CH_TOK:
        debuglog ("CH_TOK [%d, %d] [%d, %d]\n",number1, number2, row, col);
        s_row1 = number1-1;
        s_row2 = number2-1;
        debuglog ("CH_TOK scroll region %d - %d\n",s_row1, s_row2);
        break;

      case CL_TOK:
        clearscreen ();
        row = 0;
        col = 0;
        debuglog ("CL_TOK [%d, %d]\n", row, col);
        break;

      case CM_TOK:
        screen00 = screen[0][0];
        row = number1 - 1;
        col = number2 - 1;
        debuglog ("CM_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case CR_TOK:
        /* Handle missing '--more--' between inventories  MLM 24-Jun-83 */
        /* --more-- doesn't seem too be missing anymore NYM 3/29/08
         * if (row==0 && screen[0][1]==')' && screen[0][col-1] != '-')
         *   terpmes ();
         */
        col = 0;
        debuglog ("CR_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case ER_TOK:
        break;

      case LF_TOK:
        row++;
        debuglog ("LF_TOK      check for scroll %d > %d\n",row, s_row2);

        if (row > s_row2) {
          debuglog ("LF_TOK      scroll up\n");
          scrollup ();
        }

        debuglog ("LF_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case ND_TOK:
        row += number1;
        debuglog ("ND_TOK [%2d] [%2d, %2d] [%c]\n", number1, row, col, screen[row][col]);
        break;

      case SE_TOK:
        debuglog ("SE_TOK\n");
        revvideo = false;
        standend ();
        break;

      case SO_TOK:
        debuglog ("SO_TOK\n");
        revvideo = true;
        standout ();
        break;

      case TA_TOK:
        col = 8 * (1 + col / 8);
        debuglog ("TA_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case EOF:
        if (interrupted) return;

        if (!replaying || !logdigested) {
	  playing = false;
	  return;
	}

        saynow ("End of game log, type 'Q' to exit.");
        return;
        break;

      case UP_TOK:
        row--;
        debuglog ("UP_TOK      [%2d, %2d] [%c]\n", row, col,screen[row][col]);
        break;

      case HM_TOK:
        col = 0;
        row = 0;
        debuglog ("HM_TOK      [%2d, %2d] [%c]\n", row, col, screen[row][col]);
        break;

      case NU_TOK:
        row -= number1;
        debuglog ("NU_TOK [%2d] [%2d, %2d] [%c]\n", number1, row, col, screen[row][col]);
        break;

      case NR_TOK:
        col += number1;
        debuglog ("NR_TOK [%2d] [%2d, %2d] [%c]\n", number1, row, col, screen[row][col]);
        break;

      case NL_TOK:
        debuglog ("NL_TOK\n");
        col -= number1;
        debuglog ("NL_TOK [%2d] [%2d, %2d] [%c]\n", number1, row, col, screen[row][col]);
        break;

      case SC_TOK:
        debuglog ("SC_TOK      [%2d, %2d]\n", row, col);
        break;

      case RC_TOK:
        debuglog ("RC_TOK      [%2d, %2d]\n", row, col);
        break;

      case SR_TOK:
        debuglog ("SR_TOK      [%2d, %2d]\n", row, col);
        scrolldown ();
        break;

      default:
        if (ch < ' ') {
          saynow ("Unknown character '\\%o'--more--", ch);
          waitforspace ();
        }
        else if (row) {
          at (row, col);

          if (!emacs && !terse) {
	    addch (ch);
	  }

          if (row == R-1) {
	    botprinted = true;
	  } else {
	    updatepos (ch, row, col);
	  }
        }
        else if (col == 0) {
	  screen00 = screen[0][0];
	} else if (col == 1 && ch == 'l' && screen[0][0] == 'I') {
          screen[0][0] = screen00;

          if (screen00 != ' ') {
	    terpmes ();
	  }

          screen[0][0] = 'I';
        }

        screen[row][col++] = ch;
        debuglog ("OTHER   [%c] [%2d, %2d] [%c]\n", ch, row, (col-1), screen[row][col-1]);
        break;
    }
  }

  if (botprinted) terpbot ();

  if (atrow != atrow0 || atcol != atcol0) {
    updateat ();	/* Changed position, record the move */
    moved = true;	/* Indicate that we moved */
    wakemonster (8);	/* Wake up adjacent mean monsters */
    currentrectangle();	/* Keep current rectangle up to date.   LGCH */
  }

  if (!usesynch && !pending ()) {
    usesynch = true;
    lastobj = NONE;
    resetinv();
  }

  if (version < RV53A && checkrange && !pending ()) {
    command (T_OTHER, "Iz");
    checkrange = false;
  }

  /* If mapping status has changed */
  if (wasmapped != didreadmap) {
    dwait (D_CONTROL | D_SEARCH, __func__, "wasmapped: %d != didreadmap: %d",
           wasmapped, didreadmap);

    mapinfer ();
  }

  if (didreadmap != Level) {
    doors = doorlist;

    while (doors != newdoors) {
      r = *doors++; c = *doors++;
      dwait (D_INFORM, __func__, "new door at (%d,%d)", r, c);
      inferhall (r, c);
    }
  }

  if (!blinded)
    for (i = atrow-1; i <= atrow+1; i++)         /* For blanks around the  */
      for (j = atcol-1; j <= atcol+1; j++)       /* rogue...               */
        if (valrc (i,j) && seerc(' ',i,j) && onrc(CANGO,i,j)) { /* CANGO+BLANK impossible */
          unsetrc (CANGO | SAFE, i, j);          /* Infer cant go and...   */
          setnewgoal ();		         /* invalidate the map.    */
        }

  at (row, col);

  if (!emacs && !terse) {
    refresh ();
  }

  printscreen ();
  return;
}

/*
 * terpbot: Read the Rogue status line and set the various status
 * variables. This routine depends on the value of version to decide what
 * the status line looks like.
 */

void
terpbot (void)
{
  char sstr[MU_BUF + 1];	/* scanned string, +1 for paranoia */
  char modeline[SM_BUF + 1];	/* mode line, +1 for paranoia */
  int oldlev = Level, oldgold = Gold, oldhp = Hp, Str18 = 0;
  extern int geneid;
  int i, oldstr = Str, oldAc = Ac, oldExp = Explev;

  /* zeroize arrays */
  memset (sstr, 0, sizeof(sstr)); /* paranoia */
  memset (modeline, 0, sizeof(modeline)); /* paranoia */

  /* Since we use scanf to read this field, it must not be left blank */
  if (screen[R-1][C-2] == ' ') screen[R-1][C-2] = 'X';

  /* Read the bottom line, there are three versions of the status line */
  if (version < RV52A) {	/* Rogue 3.6, Rogue 4.7? */
    sscanf (screen[R-1],
            " Level: %d Gold: %d Hp: %d(%d) Str: %s Ac: %d Exp: %d/%d %s",
            &Level, &Gold, &Hp, &Hpmax, sstr, &Ac, &Explev, &Exp, Ms);
    sscanf (sstr, "%d/%d", &Str, &Str18);
    Str = Str * 100 + Str18;

    if (Str > Strmax) Strmax = Str;
  }
  else if (version < RV53A) {	/* Rogue 5.2 (versions A and B) */
    sscanf (screen[R-1],
            " Level: %d Gold: %d Hp: %d(%d) Str: %d(%d) Ac: %d Exp: %d/%d %s",
            &Level, &Gold, &Hp, &Hpmax, &Str, &Strmax, &Ac, &Explev, &Exp, Ms);

    Str = Str * 100; Strmax = Strmax * 100;
  }
  else {			/* Rogue 5.3 (and beyond???) */
    sscanf (screen[R-1],
            " Level: %d Gold: %d Hp: %d(%d) Str: %d(%d) Arm: %d Exp: %d/%d %s",
            &Level, &Gold, &Hp, &Hpmax, &Str, &Strmax, &Ac, &Explev, &Exp, Ms);

    Str = Str * 100; Strmax = Strmax * 100; Ac = 10 - Ac;
  }

  /* Monitor changes in some variables */
  if (screen[R-1][C-2] == 'X') screen[R-1][C-2] = ' ';	/* Restore blank */

  if (oldlev != Level)       newlevel ();

  if (Level > MaxLevel)      MaxLevel = Level;

  if (oldgold < Gold)        deletestuff (atrow, atcol);

  if (oldhp < Hp)            newring = true;

  lastdamage = max (0, oldhp - Hp);

  /*
   * Insert code here to monitor changes in attributes due to special
   * attacks					MLM October 26, 1983.
   */

  setbonuses ();

  /*
   * If in special output modes, generate output line
   */

  if ((oldlev != Level || oldgold != Gold || oldstr != Str ||
       oldAc != Ac || oldExp != Explev)) {
    /* Stuff the new values into the argument space (for ps command) */
    snprintf (modeline, SM_BUF, "Rgm %d: Id%d L%d %d %d(%d) s%d a%d e%d    ",
              rogpid, geneid, Level, Gold, Hp, Hpmax, Str / 100, 10-Ac, Explev);

    /* Handle Emacs and Terse mode */
    if (emacs || terse) {
      /* Skip backward over blanks and nulls */
      for (i = C-1; (i >= 0) && (screen[R-1][i] == ' ' || screen[R-1][i] == '\0'); i--);
      if (i < C-1) {
	  screen[R-1][i+1] = '\0';
      }
      screen[R-1][C] = '\0'; /* paranoia */

      if (emacs) {
        snprintf (modeline, SM_BUF, " %s (%%b)", screen[R-1]);

        if (strlen (modeline) > C-8) snprintf (modeline, SM_BUF, " %s", screen[R-1]);

        fprintf (realstdout, "%s", modeline);
        fflush (realstdout);
      }
      else if (terse && oldlev != Level) {
        fprintf (realstdout, "%s\n", screen[R-1]);
        fflush (realstdout);
      }
    }
  }
}

/*
 * dumpwalls: Dump the current screen map
 */

void
dumpwalls (void)
{
  int   r, c, S;
  char ch;

  printexplored ();

  for (r = 1; r < R-1; r++) {
    for (c = 0; c < C; c++) {
      if (valrc (r, c)) {
	S=scrmap[r][c];
	ch = (ARROW&S)                   ? 'a' :
	     (TELTRAP&S)                 ? 't' :
	     (TRAPDOR&S)                 ? 'v' :
	     (GASTRAP&S)                 ? 'g' :
	     (BEARTRP&S)                 ? 'b' :
	     (DARTRAP&S)                 ? 's' :
	     (WATERAP&S)                 ? 'w' :
	     (TRAP&S)                    ? '^' :
	     (STAIRS&S)                  ? '>' :
	     (RUNOK&S)                   ? '%' :
	     (((DOOR+BEEN)&S)==DOOR+BEEN)  ? 'D' :
	     (DOOR&S)                    ? 'd' :
	     (((BOUNDARY+BEEN)&S)==BOUNDARY+BEEN) ? 'B' :
	     (((ROOM+BEEN)&S)==ROOM+BEEN)  ? 'R' :
	     (BEEN&S)                    ? ':' :
	     (HALL&S)                    ? '#' :
	     (((BOUNDARY+WALL)&S)==BOUNDARY+WALL) ? 'W' :
	     (BOUNDARY&S)                ? 'b' :
	     (ROOM&S)                    ? 'r' :
	     (CANGO&S)                   ? '.' :
	     (WALL&S)                    ? 'W' :
	     (S)                         ? 'X' : '\0';

	if (ch) mvaddch (r, c, ch);
      }
    }
  }

  if (valrc (row, col)) {
    at (row, col);
  }
}

/*
 * sendnow: Send a string to the Rogue process.
 */

void
sendnow (char *f, ...)
{
  char cmd[MU_BUF + 1];	/* command string, +1 for paranoia */
  char *s = cmd;
  va_list ap;

  /* zeroize arrays */
  memset (cmd, 0, sizeof(cmd)); /* paranoia */

  /* setup stdarg */
  va_start (ap, f);
  vsnprintf (cmd, MU_BUF, f, ap);
  va_end (ap);

  while (*s) sendcnow (*s++);
}

/*
 * sendcnow: send a character to the Rogue process. This routine also does
 * the logging of characters in echo mode.
 */

static void
sendcnow (char c)
{
  if (replaying)
    return;

  /* i adjust the constants to fit my specific machine:
      - so i can watch at higher levels (otherwise it's too fast) and
      - so that at lower levels i want my fan speed to stay low.

     if you want to run full blast, make sure the USLEEP global
     constant is 0. */

  if ((USLEEP) && (!noterm)) {
    if (Level > 20) {
	usleep (USLEEP+(Level * 8000));
    } else if (Level > 16) {
	usleep (USLEEP+(Level * 4000));
    } else if (Level > 12) {
	usleep (USLEEP+(Level * 2000));
    } else {
	usleep (USLEEP);
    }
  }

  rogue_log_write_command (c);

  fprintf (trogue, "%c", c);
}

/*
 * send: add a string to the queue of commands to be sent to Rogue. The
 * commands are sent one at a time by the resend routine.
 */

# define bump(p,sizeq) (p)=((p)+1)%sizeq

/* VARARGS1 */
void
rogo_send (char *f, ...)
{
  char cmd[MU_BUF + 1]; /* command string, +1 for paranoia */
  char *s = cmd;
  va_list ap;

  /* zeroize arrays */
  memset (cmd, 0, sizeof(cmd)); /* paranoia */

  va_start (ap, f);
  vsnprintf (s, MU_BUF, f, ap);
  va_end (ap);

  for (; *s; bump (tail, SENDQ))
    queue[tail] = *(s++);

  /* Appends NUL, so resend will treat as a unit */
  queue[tail] = '\0';
  bump (tail, SENDQ);
}

/*
 * resend: Send next block of characters from the queue
 */

int
resend (void)
{
  char *l=lastcmd;			/* Ptr into last command */

  morecount = 0;			/* Clear message count */

  if (head == tail) return (0);		/* Fail if no commands */

  /* Send all queued characters until the next queued NULL */
  while (queue[head])
    { sendcnow (*l++ = queue[head]); bump (head, SENDQ); }

  bump (head, SENDQ);
  *l = '\0';

  return (1);				/* Return success */
}

/*
 * pending: Return true if there is a command in the queue to be sent to
 * Rogue.
 */

static int
pending (void)
{
  return (head != tail);
}

/*
 * at: move the cursor. Now just a call to move();
 */

void
at (int r, int c)
{
  if (!valrc (r, c)) {
    return;
  }

  move (r, c);
}

/*
 * deadrogue: Called when we have been killed, it reads the tombstone
 * to see how much we had when we died and who killed us. It then
 * calls quitrogue to handle the termination handshaking and log the
 * game.
 */

# define GOLDROW 15
# define KILLROW 17
# define TOMBCOL 19

static void
deadrogue (void)
{
  int    mh;
  char  *killer, *killend;

  printw ("\n\nOops... our rogue died deep with the dungeon!");
  refresh ();

  sscanf (&screen[GOLDROW][TOMBCOL], "%18d", &Gold);

  killer = &screen[KILLROW][TOMBCOL];
  killend = killer+17;

  while (*killer==' ') ++killer;

  while (*killend==' ') *(killend--) = '\0';

  /* Record the death blow if killed by a monster */
  if ((mh = findmonster (killer)) != NONE) {
    addprob (&monhist[mh].theyhit, SUCCESS);
    addstat (&monhist[mh].damage, Hp);
  }

  quitrogue (killer, Gold, DIED);
}

/*
 * quitrogue: we are going to quit. Log the game and send a \n to
 * the Rogue process, then wait for it to die before returning.
 */

/* reason - A reason string for the summary line */
/* gid - What is the final score */
/* terminationtype - SAVED, FINSISHED, or DIED */
void
quitrogue (char *reason, int gld, int terminationtype)
{
  struct tm *ts;
  long clock;
  char  *k, *r;
  int stat_loc = -1;	/* waitpid() information on rogpid */
  int options;		/* waitpid() options */
  int seconds;		/* seconds spent waiting */
  int ret;		/* waitpid() return */

  /* Save the killer and score */
  for (k=ourkiller, r=reason; *r && *r != ' '; ++k, ++r) *k = *r;

  *k = '\0';
  ourscore = gld;

  /* Don't need to make up any more commands */
  if (!replaying || !logdigested)
    playing = false;

  /* Now get the current time, so we can date the score */
  clock = time(&clock);
  ts = localtime(&clock);

  /* Build a summary line */
  memset (sumline, 0, sizeof(sumline)); /* paranoia */
  snprintf (sumline, SM_BUF, "%3s %2d, %4d %-.32s %7d%s%-17.17s %3d %3d ",
           month[ts -> tm_mon], ts -> tm_mday, 1900 + ts -> tm_year,
           getname (), gld, cheat ? "*" : " ", reason, MaxLevel, Hpmax);

  memset (sumline2, 0, sizeof(sumline2)); /* paranoia */
  if (Str % 100)
    snprintf (sumline2, BIGBUF, "%.*s%2d.%2d", SM_BUF, sumline, Str/100, Str%100);
  else
    snprintf (sumline2, BIGBUF, "%.*s  %2d ", SM_BUF, sumline, Str/100);

  memset (sumline, 0, sizeof(sumline)); /* paranoia */
  snprintf (sumline, BIGBUF, "%.*s %2d %2d/%-6d  %d",
            SM_BUF, sumline2, Ac, Explev, Exp, ltm.gamecnt);

  /* Now write the summary line to the log file */
  at (R-1, 0); clrtoeol (); refresh ();

  /* R-2 is index of score in sumline */
  if (!replaying)
    add_score (sumline, versionstr, (terse || emacs || noterm));

  /* Restore interrupt status */
  reset_int ();

  /* Set the termination message based on the termination method */
  if (stlmatch (reason, "total winner"))
    termination = "victorius";
  else if (stlmatch (reason, "user typing quit"))
    termination = "abortivus";
  else if (stlmatch (reason, "gave up"))
    termination = "inops consilii";
  else if (stlmatch (reason, "quit (scoreboard)"))
    termination = "callidus";
  else if (stlmatch (reason, "saved"))
    termination = "suspendus";

  /* Send the requisite handshaking to Rogue */
  if (terminationtype == DIED)
    if (version >= RV54A)
      sendnow ("\n\n");
    else
      sendnow ("\n");
  else if (terminationtype == FINISHED)
    sendnow ("Qy\n");
  else
    sendnow ("Syy"); /* Must send two yesses,  R5.2 MLM */

  /*
   * we need to be sure that the rogue process quit
   */

  /*
   * look for a rogue process, either running, or stopped, or a killed zombie process
   */
  if (rogpid < 0) {
      quit (1, "ERROR: %s: no rogue pid to wait for: %d\n", __func__, rogpid);
      not_reached ();
  }
  options = (WNOHANG | WUNTRACED);
  seconds = 0;
  do {

      /* look for a rogue process status */
      errno = 0;
      ret = waitpid (rogpid, &stat_loc, options);

      /* if there is not yet rogue status, wait for a second */
      if (ret == 0) {
	++seconds;
	if (seconds > ROGUE_SECONDS) {
	  break;
	}
	fprintf(stderr, "\nwaiting for rogue pid %d to exit...\n", rogpid);
	sleep(1);
      }
  } while ((ret == 0 && seconds <= ROGUE_SECONDS) || (ret < 0 && errno == EINTR));
  if (ret < 0) {
    switch (errno) {
    case EINVAL:
      quit (1, "ERROR: %s: waitpid (%d, &stat_loc, 0x%x) invalid options: %s\n",
	       __func__, rogpid, options, strerror(errno));
      not_reached ();
      break;

    case ECHILD:
      quit (1, "ERROR: %s: waitpid (%d, &stat_loc, 0x%x) cannot obtain process status: %s\n",
	       __func__, rogpid, options, strerror(errno));
      not_reached ();
      break;

    default:
      quit (1, "ERROR: %s: waitpid (%d, &stat_loc, 0x%x) returned: %d: %s\n",
	       __func__, rogpid, options, ret, strerror(errno));
      not_reached ();
      break;
    }
  }

  /*
   * analyze the status of the rogue process as returned by waitpid()
   */
  if (stat_loc == -1) {
      quit (1, "ERROR: %s: waitpid (%d, &stat_loc, 0x%x) stat_loc remains -1\n",
	       __func__, rogpid, options);
      not_reached ();
  }
  if (ret == 0 || WIFSTOPPED(stat_loc) || WIFCONTINUED(stat_loc)) {

    /* we waited ROGUE_SECONDS seconds for rogue status to become available, assume rogue is stuck and SIGHUP it */
    fprintf(stderr, "\nattempting to kill rogue pid %d with SIGHUP...\n", rogpid);
    errno = 0;
    ret = kill (rogpid, SIGHUP);
    if (ret < 0 && errno == ESRCH) {
      fprintf(stderr, "\nrogue pid %d exited before the SIGHUP attempt\n", rogpid);
      return;
    }

    /* look for a rogue process status */
    errno = 0;
    ret = waitpid (rogpid, &stat_loc, options);

    /* if there is not yet rogue status, kill SIGTERM it */
    if (ret == 0) {

      fprintf(stderr, "\nattempting to kill rogue pid %d with SIGTERM...\n", rogpid);
      errno = 0;
      ret = kill (rogpid, SIGTERM);
      if (ret < 0 && errno == ESRCH) {
	fprintf(stderr, "\nrogue pid %d exited after the SIGHUP kill attempt\n", rogpid);
	return;
      }
    }
    fprintf(stderr, "\nrogue pid %d appears to have finally exited\n", rogpid);
  }
  return;
}

/*
 * waitfor: snarf characters from Rogue until a string is found.
 *          The characters are echoed to the users screen.
 *
 *          The string must not contain a valid prefix of itself
 *          internally.
 *
 * MLM 8/27/82
 */

void
waitfor (char *mess)
{
  char *m = mess;

  while (*m) {
    if (getroguetoken () == *m) m++;
    else m = mess;
  }
}

/*
 * say: Display a messsage on the top line. Restore cursor to Rogue.
 */

void
say (char *f, ...)
{
  char buf[BUFSIZ + 1]; /* message buffer, +1 for paranoia */
  char *b;
  va_list ap;

  if (!emacs && !terse) {
    memset (buf, 0, sizeof(buf)); /* paranoia */
    va_start (ap, f);
    vsnprintf (buf, BUFSIZ, f, ap);
    va_end (ap);

    at (0,0);

    for (b=buf; *b; b++) printw ("%s", unctrl (*b));

    clrtoeol ();
    at (row, col);
  }
}

/*
 * saynow: Display a message on the top line. Restore cursor to Rogue,
 *         and refresh the screen.
 *
 * NOTE: Special case: f == NULL ==> do not format / "say" anything, just refresh the screen.
 */

void
saynow (char *f, ...)
{
  char buf[BUFSIZ + 1]; /* message buffer, +1 for paranoia */
  char *b;
  va_list ap;

  if (!emacs && !terse) {
    memset (buf, 0, sizeof(buf)); /* paranoia */
    if (f != NULL) {
      va_start (ap, f);
      vsnprintf (buf, BUFSIZ, f, ap);
      va_end (ap);
    }

    at (0,0);

    for (b=buf; *b; b++) printw ("%s", unctrl (*b));

    clrtoeol ();
    at (row, col);
  }
  refresh ();
}

/*
 * waitforspace: Wait for the user to type a space.
 * Be sure to interpret a snapshot command, if given.
 */

static void
waitforspace (void)
{
  char ch;

  refresh ();

  if (!noterm)
    while ((ch = fgetc (stdin)) != ' ')
      if (ch == '/') dosnapshot ();

  at (row, col);
}

/*
 * givehelp: Each time a ? is pressed, this routine prints the next
 * help message in a sequence of help messages. Nexthelp is an
 */

static char *nexthelp[] = {
  "Rgm commands: t=toggle run mode, e=logging, i=inventory, -=status    [?]",
  "Rgm commands: <ret>=singlestep, `=summary, /=snapshot, R=replay      [?]",
  "Rgm commands: m=long term memory display, G=display gene settings    [?]",
  "Rogue cmds: S=Save, Q=Quit, h j k l H J K L b n u y N B U Y f s < >  [?]",
  "Wizard: d=debug, !=show items, @=show monsters, #=show level flags   [?]",
  "Wizard: ~=version, ^=bowrank, %%=armorrank, $=weaponrank, ==ringrank  [?]",
  "Wizard: (=database, )=cycles, +=possible secret doors, :=chicken     [?]",
  "Wizard: [=weapstat, ]=rustproof armor, r=resetinv, &=object count    [?]",
  "Wizard: *=toggle blind, C=toggle cosmic, M=mazedoor, A=attempt, {=flags",
  NULL
};

static char **helpline = nexthelp;

void
givehelp (void)
{
  if (*helpline == NULL) helpline = nexthelp;

  saynow ("%s", *helpline++);
}

/*
 * pauserogue: Wait for the user to type a space and then redraw the
 *             screen. Now uses the stored image and passes it to
 *             curses rather than sending a form feed to Rogue. MLM
 */

void
pauserogue (void)
{
  at (R-1, 0);
  addstr ("--press space to continue--");
  clrtoeol ();
  refresh ();

  waitforspace ();

  redrawscreen ();
}

/*
 * getrogver: Read the output of the Rogue version command
 *            and set version. RV36B = 362 (3.6 with wands)
 *            and RV52A = 521 (5.2). Note that RV36A is
 *            infered when we send a "//" command to identify
 *            wands.
 *
 * Get version from first 2000 chars of a log file	Feb 9, 1985 - mlm
 */

# define VERMSG	"ersion "

void
getrogver (void)
{
  char *vstr = versionstr, *m = VERMSG;
  int cnt = 2000, ch;

  if (replaying) {		/* Look for version string in log */
    while (cnt-- > 0 && *m)
      { if (fgetc (logfile) == *m) m++; else m = VERMSG;}

    if (*m == '\0') {		/* Found VERMSG, get version string */
      while ((ch = fgetc (logfile)) != ' ') *(vstr++) = ch;

      *--vstr = '\0';
    }
    else {				/* Use default version */
      memset (versionstr, 0, sizeof(versionstr)); /* paranoia */
      snprintf (versionstr, MU_BUF, DEFVER);
    }

    rewind (logfile);			/* Put log file back to start */
  }

  else {				/* Execute the version command */
    sendnow ("v");
    waitfor ("ersion ");

    while ((ch = getroguetoken ()) != ' ') {
      *vstr = ch;
      vstr++;
    }
  }

  if (stlmatch (versionstr, "3.6"))		version = RV36B;
  else if (stlmatch (versionstr, "5.2"))	version = RV52A;
  else if (stlmatch (versionstr, "5.3"))	version = RV53A;
  else if (stlmatch (versionstr, "5.4.4"))	version = RV54A;
  else if (stlmatch (versionstr, "5.4.5"))	version = RV54B;
  else {

    /*
     * unable too parse the rogue version
     */
    saynow ("What a strange version of Rogue! ");

    /* use the default rogue version number */
    version = DEFRV;

    /* use the default rogue version string */
    memset (versionstr, 0, sizeof(versionstr)); /* paranoia */
    snprintf (versionstr, MU_BUF, DEFVER);
  }
}

/*
 * charsavail: How many characters are there at the terminal? If any
 * characters are found, 'noterm' is reset, since there is obviously
 * a terminal around if the user is typing at us.
 */

int
charsavail (void)
{
  long n;
  int retc;

  retc = ioctl (READ, FIONREAD, &n);
  if (retc) {
    saynow ("Ioctl returns %d, n=%ld.\n", retc, n);
    n=0;
  }

  if (n > 0) noterm = false;

  return ((int) n);
}

/*
 * redrawscreen: Make the users screen look like the Rogue screen (screen).
 */

void
redrawscreen (void)
{
  int i, j;
  char ch;

  clear ();

  for (i = 1; i < R; i++) for (j = 0; j < C; j++)
      if ((ch = screen[i][j]) > ' ') mvaddch(i, j, ch);

  at (row, col);

  refresh ();
}

/*
 * toggleecho: toggle the I/O echo feature. If first time, open the
 * roguelog file.
 */

void
toggleecho (void)
{
  if (replaying) return;

  logging = !logging;

  if (logging) {
    if (! rogue_log_open (ROGUELOG)) {
      logging = !logging;
      saynow ("can't open %s", ROGUELOG);
    }
    else {
      saynow ("Logging to file %s", ROGUELOG);

      if (*versionstr) command (T_OTHER, "v");
    }
  }
  else {
    rogue_log_close ();

    if (playing) saynow ("File %s closed", ROGUELOG);
  }

  if (playing)
    { at (row, col); refresh (); }
}

/*
 * clearsendqueue: Throw away queued Rogue commands.
 */

void
clearsendqueue (void)
{
  head = tail;
}

/*
 * startreplay: Open the log file to replay.
 */

void
startreplay (FILE **logf, char *logfname)
{
  if ((*logf = fopen (logfname, "r")) == NULL) {
    fprintf (stderr, "Can't open '%s'.\n", logfname);
    exit(1);
  }
}

/*
 * putn: Put 'n' copies of character 'c' on file 'f'.
 */

static void
putn (char c, FILE *f, int n)
{
  while (n--)
    putc (c, f);
}

/*
 * printsnap: print a snapshot to file f.
 */

void
printsnap (FILE *f)
{
  int i, j, length;
  struct tm *ts;
  long clock;

  /* Now get the current time, so we can date the snapshot */
  clock = time(&clock);
  ts = localtime(&clock);

  /* Print snapshot timestamp */
  fprintf (f, "\nSnapshot taken on %s %d, %d at %02d:%02d:%02d:\n\n",
           month[ts -> tm_mon], ts -> tm_mday, 1900 + ts -> tm_year,
           ts -> tm_hour, ts -> tm_min, ts -> tm_sec);

  /* Print the current map */
  putn ('-', f, C-1);
  fprintf (f, "\n");

  for (i = 0; i < R; i++) {
    for (length = C-1; length >= 0 && charonscreen(i,length) == ' '; length--);

    for (j=0; j <= length; j++) fprintf (f, "%c", charonscreen(i,j));

    fprintf (f, "\n");
  }

  putn ('-', f, C-1);

  /* Print status variables */
  fprintf (f, "\n\n%s\n\n", statusline ());

  /* Print the inventory */

  dumpinv (f);
  fprintf (f, "\n");
  putn ('-', f, C-1);
  fprintf (f, "\n");
}

/*
 * dosnapshot: add a snapshot to the SHAPSHOT file.
 */

void
dosnapshot (void)
{
  if ((snapshot = wopen (SNAPSHOT, "a")) == NULL)
    saynow ("Cannot write file %s.", SNAPSHOT);
  else {
    printsnap (snapshot);
    fclose (snapshot);
    saynow ("Snapshot added to %s.", SNAPSHOT);
  }
}

/*
 * clearscreen: Done whenever a {ff} is sent by Rogue.  This code is
 * separate so it can be called from replay(), since there is an implicit
 * formfeed not recorded in the log file.   MLM
 */

void
clearscreen (void)
{
  int i, j;

  row = col = 0;
  clear ();
  screen00 = ' ';

  for (i = 0; i < R; i++)
    for (j = 0; j < C; j++) {
      screen[i][j] = ' ';
      scrmap[i][j] = SCRMINIT;
    }

  initstufflist ();
  mlistlen = 0;  /* initmonsterlist (); temp hack MLM */
}

/*
 * statusline: Write all about our current status into a string.
 * Returns a pointer to a static area.			MLM
 */

char *
statusline (void)
{
  static char staticarea[BIGBUF + 1]; /* +1 for paranoia */
  static char staticarea2[BIGBUF + 1]; /* +1 for paranoia */
  char *ret;	/* static buffer to return as a NUL terminated string */

  memset (staticarea, 0, sizeof(staticarea)); /* paranoia */
  strlcpy (staticarea, "Status: ", sizeof(staticarea));

  if (aggravated)		strlcat (staticarea, "aggravated, ", sizeof(staticarea));

  if (beingheld)		strlcat (staticarea, "being held, ", sizeof(staticarea));

  if (blinded)			strlcat (staticarea, "blind, ", sizeof(staticarea));

  if (confused)			strlcat (staticarea, "confused, ", sizeof(staticarea));

  if (cosmic)			strlcat (staticarea, "cosmic, ", sizeof(staticarea));

  if (cursedarmor)		strlcat (staticarea, "cursed armor, ", sizeof(staticarea));

  if (cursedweapon)		strlcat (staticarea, "cursed weapon, ", sizeof(staticarea));

  if (doublehasted)		strlcat (staticarea, "perm hasted, ", sizeof(staticarea));

  if (droppedscare)		strlcat (staticarea, "dropped scare, ", sizeof(staticarea));

  if (diddrop)			strlcat (staticarea, "dropped something, ", sizeof(staticarea));

  if (floating)			strlcat (staticarea, "floating, ", sizeof(staticarea));

  if (hasted)			strlcat (staticarea, "hasted, ", sizeof(staticarea));

  if (protected)		strlcat (staticarea, "protected, ", sizeof(staticarea));

  if (redhands)			strlcat (staticarea, "red hands, ", sizeof(staticarea));

  if (Level == didreadmap)	strlcat (staticarea, "mapped, ", sizeof(staticarea));

  memset (staticarea2, 0, sizeof(staticarea2)); /* paranoia */
  if (*genocided)
  { snprintf (staticarea2, SM_BUF, "%.*sgenocided '%.*s', ", MU_BUF, staticarea, MU_BUF, genocided);
    snprintf (staticarea, BIGBUF, "%.*s%d food%s, %d missile%s, %d turn%s, (%d,%d %d,%d) bonus",
              SM_BUF, staticarea2, larder, plural(larder), ammo, plural(ammo), turns,
              plural(turns), gplushit, gplusdam, wplushit, wplusdam);
    ret = staticarea;
  }
  else
  { snprintf (staticarea2, BIGBUF, "%.*s%d food%.*s, %d missile%.*s, %d turn%.*s, (%d,%d %d,%d) bonus",
	      SM_BUF, staticarea, larder, MU_BUF, plural(larder), ammo, MU_BUF, plural(ammo), turns,
	      MU_BUF, plural(turns), gplushit, gplusdam, wplushit, wplusdam);
    ret = staticarea2;
  }

  return (ret);
}
