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
 * command.c:
 *
 * This file contains all of the functions which send commands to
 * Rogue, this file and 'things.c' make up the effector interface.
 */

# include <stdlib.h>
# include <ctype.h>
# include <string.h>
# include <stdarg.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "modern_curses.h"
# include "types.h"
# include "globals.h"

# define EQUAL 0

/* static declarations */
static int cmdonscreen = false;
static int commandcount (char *cmd);
static int functionesc (char *cmd);
static char commandarg (char *cmd, int n);
static void adjustpack (char *cmd);
static void bumpsearchcount (void);
static void clearcommand (void);
static void usemsg (char *str, int obj);

/* Move one square in direction 'd' */
void
move1 (int d)
{
  command (T_MOVING, "%c", keydir[d]);
}

/* Move in direction 'd' until we find something */
void
fmove (int d)
{
  if (version < RV53A)	command (T_MOVING, "f%c", keydir[d]);
  else			command (T_MOVING, "%c", ctrl (keydir[d]));
}

/* Move 'count' squares in direction 'd', with time use mode 'mode' */
void
rmove (int count, int d, int mode)
{
  command (mode, "%d%c", count, keydir[d]);
}

/* Move one square in direction 'd' without picking anything up */
void
mmove (int d, int mode)
{
  command (mode, "m%c", keydir[d]);
}

/*
 * command: Send a command which takes Rogue time to execute. These
 * include movement commands, sitting, and physical actions. Actions which
 * gather information are sent to Rogue using the 'send' function.
 */

void
command (int tmode, char *f, ...)
{
  int times;
  char cmd[MU_BUF + 1]; /* rogue command, +1 for paranoia */
  va_list ap;

  /* zeroize arrays */
  memset (cmd, 0, sizeof(cmd)); /* paranoia */

  /* Build the command */
  va_start (ap, f);
  vsnprintf (cmd, MU_BUF, f, ap);
  va_end (ap);

  debuglog ("%s: rogue cmd: (%s)\n", __func__, cmd);

  /* Echo the command if in transparent mode */
  if (transparent)		showcommand (cmd);
  else if (cmdonscreen)		clearcommand ();

  /* Figure out whether and in which direction we are moving */
  switch ((functionchar (cmd) & 037) | 0100) {
    case 'L': movedir = 0; wakemonster (movedir); break;
    case 'U': movedir = 1; wakemonster (movedir); break;
    case 'K': movedir = 2; wakemonster (movedir); break;
    case 'Y': movedir = 3; wakemonster (movedir); break;
    case 'H': movedir = 4; wakemonster (movedir); break;
    case 'B': movedir = 5; wakemonster (movedir); break;
    case 'J': movedir = 6; wakemonster (movedir); break;
    case 'N': movedir = 7; wakemonster (movedir); break;
    default:  movedir = NOTAMOVE;
  }

  /* If command takes time to execute, mark monsters as sleeping */
  /* If they move, wakemonsters will mark them as awake */
  if (tmode != T_OTHER)
    sleepmonster ();

  /* Do time accounting */
  times = commandcount (cmd);

  if (tmode < T_OTHER || tmode >= T_LISTLEN) tmode = T_OTHER;

  turns += times;
  timespent[Level].timestamp = turns;
  timespent[Level].activity[tmode] += times > 1 ? times : 1;

  /* Do the inventory stuff */
  if (movedir == NOTAMOVE)
    adjustpack (cmd);
  else
    diddrop = false;

  /* If we have a ring of searching, take that into account */
  if (wearing ("searching") != NONE)
    bumpsearchcount ();

  rogo_send ("%s", cmd);
  return;
}

/*
 * commandcount: Return the number of a times a command is to happen.
 */

static int
commandcount (char *cmd)
{
  int times = atoi (cmd);

  return (max (times, 1));
}

/*
 * functionchar: return the function character of a command.
 */

char
functionchar (char *cmd)
{
  char *s = cmd;

  while (ISDIGIT (*s) || *s == 'f') s++;

  return (*s);
}

/*
 * functionesc: return true if the next character after
 *              a function letter is an ESC.
 */

static int
functionesc (char *cmd)
{
  char *s = cmd;

  while (ISDIGIT (*s) || *s == 'f') s++;

  s++;
  return ((*s == ESC));
}

/*
 * commandarg: return the nth argument of a command.
 */

static char
commandarg (char *cmd, int n)
{
  char *s = cmd;

  while (ISDIGIT (*s) || *s == 'f') s++;

  return (s[n]);
}

/*
 * adjustpack: adjust pack in accordance with command.
 */

static void
adjustpack (char *cmd)
{
  int neww, obj;

  switch (functionchar (cmd)) {
    case 'd':	if (!diddrop) {
      setrc (STUFF | USELESS, atrow, atcol);
      deleteinv (OBJECT (commandarg (cmd, 1)));
      }
      break;

    case 'e':   removeinv (OBJECT (commandarg (cmd, 1)));
      Ms[0] = 'X'; newring = true;
      lastate = turns;
      break;

    case 'i':	doresetinv ();
      break;

    case 'q':	lastobj = OBJECT (commandarg (cmd, 1));
      usemsg ("Quaffing", lastobj);
      strlcpy (lastname, inven[lastobj].str, sizeof(lastname));
      useobj (inven[lastobj].str);
      removeinv (lastobj);
      break;

    case 'r':	lastobj = OBJECT (commandarg (cmd, 1));
      usemsg ("Reading", lastobj);
      strlcpy (lastname, inven[lastobj].str, sizeof(lastname));
      useobj (inven[lastobj].str);
      removeinv (lastobj);
      break;

    case 't':	removeinv (OBJECT (commandarg (cmd, 2)));
      hitstokill -= 1; /* Don't blame weapon if arrow misses */
      break;

    case 'w': if (!functionesc (cmd)) {
      if (currentweapon != NONE)
        forget (currentweapon, INUSE);

      neww = OBJECT (commandarg (cmd, 1));
      usemsg ("About to wield", neww);

      if (commandarg (cmd, 2) == 'w')
        { lastdrop = currentweapon = neww; }
      else
        { lastdrop = currentweapon; currentweapon = neww; }

      remember (currentweapon, INUSE);

      usingarrow = (inven[currentweapon].type == missile);
      goodweapon = (weaponclass (currentweapon) >= 100);

      badarrow = false;
      goodarrow = false;
      poorarrow = false;
      hitstokill = false;
      newweapon = true;
      setbonuses ();
      }
      break;

    case 'p': case 'z':
      lastwand = OBJECT (commandarg (cmd, 2));
      usemsg ("Pointing", lastwand);
      strlcpy (lastname, inven[lastwand].str, sizeof(lastname));
      useobj (inven[lastwand].str);

      /* Update number of charges */
      if (inven[lastwand].charges > 0) {
        if (version >= RV52A &&
            stlmatch (inven[lastwand].str, "striking"))
          inven[lastwand].charges -= 2;
        else
          inven[lastwand].charges--;
      }

      hitstokill -= 1; /* Don't blame weapon if wand misses */
      break;

    case 's':   bumpsearchcount ();
      break;

    case 'P':	obj = OBJECT (commandarg (cmd, 1));
      usemsg ("Putting on", obj);

      if (commandarg (cmd, 2) == 'l')		leftring = obj;
      else if (commandarg (cmd, 2) == 'r')	rightring = obj;
      else if (leftring == NONE)		leftring = obj;
      else					rightring = obj;

      /* Check for putting on see invisible */
      if (streq (inven[obj].str, "see invisible"))
        { beingstalked = 0; putonseeinv = turns; }

      remember (obj, INUSE);
      setbonuses ();
      newarmor = true;

      break;

    case 'R':	if (commandarg (cmd, 1) == 'l')
        { lastdrop = leftring; leftring = NONE; }
      else if (commandarg (cmd, 1) == 'r')
        { lastdrop = rightring; rightring = NONE; }
      else if (leftring != NONE)
        { lastdrop = leftring; leftring = NONE; }
      else
        { lastdrop = rightring; rightring = NONE; }

      usemsg ("Taking off", lastdrop);

      forget (lastdrop, INUSE);
      setbonuses ();
      newarmor = true;

      break;

    case 'T':   lastdrop = currentarmor;
      usemsg ("About to take off", currentarmor);
      forget (currentarmor, INUSE);
      currentarmor = NONE;
      newarmor = true;
      break;

    case 'W':	currentarmor = OBJECT (commandarg (cmd, 1));
      usemsg ("About to wear", currentarmor);
      remember (currentarmor, INUSE);
      newarmor = true;
      break;
  }
}

/*
 * bumpsearchcount: Note that we just searched this square.
 */

static void
bumpsearchcount (void)
{
  int dr, dc;

  for (dr = -1; dr <= 1; dr++)
    for (dc = -1; dc <= 1; dc++)
      timessearched[atrow+dr][atcol+dc]++;
}

/*
 * replaycommand: Find the old command in the log file and send it.
 */

int
replaycommand (void)
{
  char oldcmd[128];

  getoldcommand (oldcmd);
  command (T_OTHER, "%s", oldcmd);
  return (1);
}

/*
 * showcommand:		Echo a string in the lower right hand corner.
 * clearcommand:	Remove the command we showed.
 */

void
showcommand (char *cmd)
{
  char *s;
  int i = 72;

  at (23,72); standout (); printw (" ");

  for (s=cmd; *s; s++) {
    if ((i + strlen (unctrl(*s))) < 78) {
      printw ("%s", unctrl (*s));
    }
    i += strlen (unctrl(*s));
  }

  printw (" "); standend (); clrtoeol (); at (row, col); refresh ();
  cmdonscreen = true;
}

static void
clearcommand (void)
{
  at (23,72); clrtoeol (); at (row, col);
  cmdonscreen = false;
}

/*
 * usemsg: About to use an item, tell the user.
 */

static void
usemsg (char *str, int obj)
{
  if (! dwait (D_INFORM, __func__, "%s (%s", str, itemstr (obj)))
    saynow ("%s (%s", str, itemstr (obj));
}

