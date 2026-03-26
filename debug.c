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
 * debug.c:
 *
 * This file contains the code for the debugger.  Rogomatic has one of
 * the tensest internal debuggers around, because in the early days it
 * had an incredible number of bugs, with no way to repeat an error
 * (because Rogue uses a different dungeon each time).
 */

# include <stdlib.h>
# include <setjmp.h>
# include <string.h>
# include <stdarg.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "modern_curses.h"
# include "types.h"
# include "globals.h"
# include "install.h"

/* static declarations */
static void dumpflags (int r, int c);
static int getscrpos (char *msg, int *r, int *c);
static const char *msgtype_str (int msgtype);

/*
 * msgtype_str: message type as a string
 */
static const char *
msgtype_str(int msgtype)
{
    switch (msgtype & D_ALL) {
    case D_FATAL:
	return "FATAL";
	break;
    case D_ERROR:
	return "ERROR";
	break;
    case (D_WARNING | D_SAY):
	return "Warn-say";
	break;
    case D_WARNING:
	return "Warning";
	break;
    case D_INFORM:
	return "info";
	break;
    case (D_CONTROL | D_SAY):
	return "ctrl-say";
	break;
    case (D_CONTROL | D_SEARCH):
	return "ctrl-srch";
	break;
    case D_SEARCH:
	return "search";
	break;
    case (D_BATTLE | D_MONSTER):
	return "bat-monst";
	break;
    case (D_BATTLE | D_SEARCH):
	return "bat-srch";
	break;
    case D_BATTLE:
	return "battle";
	break;
    case D_MESSAGE:
	return "msg";
	break;
    case D_PACK:
	return "pack";
	break;
    case D_CONTROL:
	return "ctrl";
	break;
    case D_SCREEN:
	return "screen";
	break;
    case D_MONSTER:
	return "monster";
	break;
    case D_SAY:
	return "say";
	break;
    default:
	break;
    }
    return "other";
}

/*
 * Debugging wait loop: Handle the usual Rogomatic command chars, and also
 * allows dumping the flags '^' command. Exits when a non-command char is
 * typed. To use, just put a "dwait (type, __func__, "message");" wherever you need
 * debugging messages, and hit a space or a cr to continue
 */

int
dwait(int msgtype, const char *from, char *f, ...)
{
  char msg[MU_BUF + 1];	/* message buffer, +1 for paranoia */
  int from_len;		/* length of function name by colon (:) space */
  int r, c;
  va_list ap;

  /* zeroize arrays */
  memset (msg, 0, sizeof(msg)); /* paranoia */

  /* pre-load calling function name followed by colon (:) space */
  from_len = strlen(from) + 1 + 1;
  snprintf(msg, from_len+1, "%s: ", from);

  /* Build the actual message */
  va_start (ap, f);
  vsnprintf (msg+from_len, MU_BUF-from_len, f, ap);
  va_end(ap);

  /* Log the message if the error is severe enough */
  if (!replaying && (msgtype & (D_FATAL | D_ERROR | D_WARNING))) {
    char *path;	    /* error filename */
    FILE *errfil;

    /* form error filename */
    path = form_prefix_path (rgmdir, "error",  versionstr);

    /* append an error message to the error filename */
    if ((errfil = wopen (path, "a")) != NULL) {
      fprintf (errfil, "User %s: error type %d (%s): %s\n\n",
               getname(), msgtype, msgtype_str(msgtype), msg);

      if (msgtype & (D_FATAL | D_ERROR)) {
        printsnap (errfil);
        summary (errfil, NEWLINE);
        fprintf (errfil, "\f\n");
      }

      fclose (errfil);
    }

    /* free error filename */
    if (path != NULL) {
	free(path);
	path = NULL;
    }
  }

  if (msgtype & D_FATAL) {
    extern jmp_buf commandtop;			/* From play */
    saynow (msg);
    playing = false;
    quitrogue (msg, Gold, SAVED);
    longjmp (commandtop, 0);
  }

  if (! debug (msgtype | D_INFORM)) {	/* If debugoff */
    if (msgtype & D_SAY)			  /* Echo? */
      { saynow (msg); va_end (ap); return (1); }  /* Yes => win */

    return (0);					  /* No => lose */
  }

  if (*msg) { mvaddstr (0, 0, msg); clrtoeol (); }	/* Write msg */

  if (noterm) { va_end (ap); return (1); }		/* Exit if no user */

  /* Debugging loop, accept debugging commands from user */
  while (1) {
    refresh ();

    switch (fgetc (stdin)) {
      case '?':
        say ("i=inv, d=debug !=stf, @=mon, #=wls, $=id, ^=flg, &=chr");
        break;
      case 'i': at (1,0); dumpinv ((FILE *) NULL); at (row, col); break;
      case 'd': toggledebug ();		break;
      case 't': transparent = true;        break;
      case '!': dumpstuff ();           break;
      case '@': dumpmonster ();         break;
      case '#': dumpwalls ();           break;
      case '^': promptforflags ();	break;
      case '&':

        if (getscrpos ("char", &r, &c))
          saynow ("Char at %d,%d '%c'", r, c, screen[r][c]);

        break;
      case '(': dumpdatabase (); at (row, col); break;
      case ')': new_mark = true; markcycles (DOPRINT); at (row, col); break;
      case '~': saynow ("Version %d", version); break;
      case '/': dosnapshot (); break;
      default: at (row, col); va_end (ap); return (1);
    }
  }
}

/*
 * promptforflags: Prompt the user for a location and dump its flags.
 */

void
promptforflags (void)
{
  int r, c;

  if (getscrpos ("flags", &r, &c)) {
    mvprintw (0, 0, "Flags for %d,%d ", r, c);
    dumpflags (r, c);
    clrtoeol ();
    at (row, col);
  }
}

/*
 * dumpflags: Create a message line for the scrmap flags of a particular
 *            square.  Note that the fnames[] array must match the
 *            various flags defined in "types.h".
 */

static char *fnames[] = {
  "been",    "cango",    "door",     "hall",     "psd",     "room",
  "safe",    "seen",     "deadend",  "stuff",    "trap",    "arrow",
  "trapdor", "teltrap",  "gastrap",  "beartrap", "dartrap", "waterap",
  "monster", "wall",     "useless",  "scarem",   "stairs",  "runok",
  "boundry", "sleeper",  "everclr"
};

static void
dumpflags (int r, int c)
{
  char **f; int b;

  printw (":");

  for (f=fnames, b=1;   b<=EVERCLR;   b = b * 2, f++)
    if (scrmap[r][c] & b)
      printw ("%s:", *f);
}

/*
 * Timehistory: print a time analysis of the game.
 */

void
timehistory (FILE *f, char sep)
{
  int i, j;
  char s[BUFSIZ + 1];	/* time history message, +1 for paranoia */
  char s2[MU_BUF + 1];	/* level message, +1 for paranoia */

  /* zeroize arrays */
  memset (s, 0, sizeof(s)); /* paranoia */

  timespent[0].timestamp = 0;

  snprintf (s, BUFSIZ, "Time Analysis: %s%c%c",
           "othr hand fght rest move expl rung grop srch door total",
           sep, sep);

  for (i=1; i<=MaxLevel; i++) {
    memset (s2, 0, sizeof(s2)); /* paranoia */
    snprintf (s2, MU_BUF, "level %2d:     ", i);
    strlcat (s, s2, sizeof(s));
    for (j = T_OTHER; j < T_LISTLEN; j++) {
      snprintf (s2, MU_BUF, "%5d", timespent[i].activity[j]);
      strlcat (s, s2, sizeof(s));
    }
    snprintf (s2, MU_BUF, "%6d%c",
             timespent[i].timestamp - timespent[i-1].timestamp, sep);
    strlcat (s, s2, sizeof(s));
  }

  if (f == NULL)
    addstr (s);
  else
    fprintf (f, "%s", s);
}

/*
 * toggledebug: Set the value of the debugging word.
 */

void
toggledebug (void)
{
  char debugstr[MU_BUF + 1];
  int type = debugging & ~(D_FATAL | D_ERROR | D_WARNING);

  /* zeroize arrays */
  memset (debugstr, 0, sizeof(debugstr));

  if (debugging == D_ALL)         debugging = D_NORMAL;
  else if (debugging == D_NORMAL) debugging = D_NORMAL | D_SEARCH;
  else if (type == D_SEARCH)      debugging = D_NORMAL | D_BATTLE;
  else if (type == D_BATTLE)      debugging = D_NORMAL | D_MESSAGE;
  else if (type == D_MESSAGE)     debugging = D_NORMAL | D_PACK;
  else if (type == D_PACK)        debugging = D_NORMAL | D_MONSTER;
  else if (type == D_MONSTER)     debugging = D_NORMAL | D_CONTROL;
  else if (type == D_CONTROL)     debugging = D_NORMAL | D_SCREEN;
  else if (type == D_SCREEN)      debugging = D_NORMAL | D_WARNING;
  else if (!debug (D_INFORM))     debugging = D_NORMAL | D_WARNING | D_INFORM;
  else                            debugging = D_ALL;

  strlcpy (debugstr, "Debugging :", sizeof(debugstr));

  if (debug(D_FATAL))     strlcat (debugstr, "fatal:", sizeof(debugstr));

  if (debug(D_ERROR))     strlcat (debugstr, "error:", sizeof(debugstr));

  if (debug(D_WARNING))   strlcat (debugstr, "warn:", sizeof(debugstr));

  if (debug(D_INFORM))    strlcat (debugstr, "info:", sizeof(debugstr));

  if (debug(D_SEARCH))    strlcat (debugstr, "search:", sizeof(debugstr));

  if (debug(D_BATTLE))    strlcat (debugstr, "battle:", sizeof(debugstr));

  if (debug(D_MESSAGE))   strlcat (debugstr, "msg:", sizeof(debugstr));

  if (debug(D_PACK))      strlcat (debugstr, "pack:", sizeof(debugstr));

  if (debug(D_CONTROL))   strlcat (debugstr, "ctrl:", sizeof(debugstr));

  if (debug(D_SCREEN))    strlcat (debugstr, "screen:", sizeof(debugstr));

  if (debug(D_MONSTER))   strlcat (debugstr, "monster:", sizeof(debugstr));

  saynow (debugstr);
}

/*
 * getscrpos: Prompt the user for an x,y coordinate on the screen.
 */

static int
getscrpos (char *msg, int *r, int *c)
{
  char buf[256];

  saynow ("At %d,%d: enter 'row,col' for %s: ", atrow, atcol, msg);

  if (fgets (buf, 256, stdin)) {
    sscanf (buf, "%d,%d", r, c);

    if (*r>=1 && *r<23 && *c>=0 && *c<=79)
      return (1);
    else
      say ("%d,%d is not on the screen!", *r, *c);
  }

  at (row, col);
  return (0);
}
