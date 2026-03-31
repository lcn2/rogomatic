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
 * mess.c:
 *
 * This file contains all of the functions which parse the message line.
 */

# include <stdlib.h>
# include <ctype.h>
# include <string.h>

# include "have_strlcat.h"
# include "have_strlcpy.h"
# include "strl.h"
# include "modern_curses.h"
# include "types.h"
# include "globals.h"

/* Matching macros */
# define MATCH(p) smatch(mess,p,result)

/* static declarations */

/* Local data recording statistics */
static int monkilled[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static int totalkilled=0, timeshit=0, timesmissed=0, hits=0, misses=0;
static int sumgold=0, sumsqgold=0, numgold=0;

static int mhit=0, mmiss=0, mtarget= NONE;

/* Other local data */
static bool identifying = false;  /* True if message is from identify scroll */
static bool justreadid = false;	  /* True if just read identify scroll */
static bool gushed = false;	  /* True ==> water on the head msg recently */
static bool echoit = false;	  /* True ==> echo this message to the user */

/* Results from star matcher */
static char res1[NAMSIZ], res2[NAMSIZ], res3[NAMSIZ], res4[NAMSIZ], res5[NAMSIZ];
static char *result[] = { res1, res2, res3, res4, res5 };

static void parsemsg (char *mess, char *mend);
static int smatch (char *dat, char *pat, char **res);
static void readident (char *name);
static void rampage (void);
static void curseditem (void);
static void washit (char *monster);
static void wasmissed (char *monster);
static void didhit (void);
static void didmiss (void);
static void mshit (char *monster);
static void msmiss (char *monster);
static void countgold (char *amount);
static int getmonhist (char *monster, int hitormiss);

/*
 * terpmes: called when a message from Rogue is on the top line,
 * this function parses the message and notes the information.
 * Note that the messages are all lower cased, to help with
 * compatability bewtween 3.6 and 5.2, since 5.2 capitalizes more
 * messages than does 3.6.  Trailing punctuation is also ignored.
 *
 * As of Rogue 5.3, multiple messages are broken into single
 * messages before being passed to parsemsg.  Periods separate
 * multiple messages.
 */

void
terpmes (void)
{
  char mess[MU_BUF + 1];
  char topline[MU_BUF + 1];
  char *m, *mend, *s, *t;

  /* zeroize arrays */
  memset (topline, 0, sizeof(topline));

  s=&screen[0][0];
  strlcpy (topline, s, sizeof(topline));
  s=topline;

  /* Set 't' to the tail of the message,
      skip backward until you find a letter, digit, or punctuation */
  t=topline+C-1;

  while ((isspace(*t) || *t == '.' || *t == '-') && (t > topline)) {
    if (*t == '-' || *t == '.' || *t == '\0')
      *t = ' ';

    t--;
  }

  t++;  /* t -> beyond string */

  /*
   * Loop through each message, finding the beginning and end, and
   * copying it to mess, lower-casing it as we go. Then call parsemsg.
   */

  while (s<t) {				      /* While more chars in msg */
    memset (mess, 0, sizeof(topline));

    while (*s==' ' && s<t) s++;			/* Skip leading blanks */

    for (m = mess;				/* Copy text */
         s<t && (version < RV53A || *s != '.' || s[1] != ' ');
         s++)	{
      if (isprint(*s))
        *m = isupper (*s) ? tolower (*s) : *s;	  /* Lower case the char */

      m++;
    }

    s++;					/* Skip the period, if any */

    mend = m;

    /* :ANT: for debugging screen now has to be at least 31x80 */
    if debug(D_MESSAGE) {
      at (R,0);
      printw (">%-*.*s", C-1, C-1, screen);
      at (R+1,0);
      printw (">%-*.*s", C-1, C-1, topline);
      at (R+2,0);
      clrtoeol ();
      printw (">%-*.*s", C-1, C-1, mess);
      refresh ();
    }

    /* :ANT: */

    if (mess != mend) parsemsg (mess, mend);	/* Parse it */

    /* :ANT: for debugging */
    if debug(D_MESSAGE) {
      at (R,0);
      printw ("<%-*.*", C-1, C-1, screen);
      at (R+1,0);
      printw ("<%-*.*", C-1, C-1, topline);
      at (R+2,0);
      clrtoeol ();
      printw ("<%-*.*", C-1, C-1, mess);
      refresh ();
    }

    /* :ANT: */
  }
}

/*
 * parsemsg: Parse a single message, and if necessary set variables
 * or call functions.
 */

static void
parsemsg (char *mess, char *mend)
{
  bool unknown = false;

  echoit = true;

  /*----------------Take action based on type of message-------------*/

  /* :ANT: let's tag this as a BEARTRP for now */
  if (MATCH("* sparks dance across your armor*"))
    nametrap (BEARTRP,HERE);

  /* :ANT: */

  /* nymph stole an item, pack is out of whack */
  else if (MATCH("she stole *")) {
    usesynch = false;
  }

  /* Message indicates we picked up a new item */
  else if (*(mend-1)==')' && *(mend-3)=='(') {
    inventory (mess, mend);
    identifying = false;
    justreadid = false;
    usesynch = false;
  }
  /* Message describes an old item already in our pack */
  else if (mess[1]==')') {
    echoit = identifying;
    identifying = false;
    justreadid = false;
    inventory (mess, mend);
  }
  /* A random message, switch of first char to save some time... */
  else switch (mess[0]) {
      case 'a':

        if (MATCH("a secret door*")) echoit = false;
        else if (MATCH("as you read the scroll, it vanishes*")) echoit = false;
        else if (MATCH("a cloak of darkness falls around you*"))
          { infer ("blindness", potion); blinded = true; }
        else if (MATCH("a teleport trap*")) nametrap (TELTRAP,NEAR);
        else if (MATCH("a trapdoor*")) nametrap (TRAPDOR,NEAR);
        else if (MATCH("an arrow shoots *"))
          { arrowshot = true; nametrap(ARROW,HERE); }
        else if (MATCH("an arrow trap*")) nametrap (ARROW,NEAR);
        else if (MATCH("a beartrap*")) nametrap (BEARTRP,NEAR);
        else if (MATCH("a strange white mist *")) nametrap (GASTRAP,HERE);
        else if (MATCH("a sleeping gas trap*")) nametrap (GASTRAP,NEAR);
        else if (MATCH("a small dart *")) nametrap (DARTRAP,HERE);
        else if (MATCH("a dart trap*")) nametrap (DARTRAP,NEAR);
        else if (MATCH("a poison dart trap*")) nametrap (DARTRAP,NEAR);
        else if (MATCH("a rust trap*")) nametrap (WATERAP,NEAR);
        else if (MATCH("a gush of water hits you on the head*")) gushed = true;
        else if (MATCH("a sting has weakened you*")) ;
        else if (MATCH("a bite has weakened you*")) ;

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("a mysterious trap*")) nametrap (BEARTRP,NEAR);
        else if (MATCH("a spike shoots past your ear!*")) nametrap (BEARTRP,HERE);
        else if (MATCH("a * light flashes in your eyes*")) nametrap (BEARTRP,HERE);

        /* :ANT: */

        else if (MATCH("a new monster is nearby*")) infer ("create monster", Scroll);
        else if (MATCH("a staff of * [*](*)*")) infer (res1, wand);
        else if (MATCH("a wand of * [*](*)*")) infer (res1, wand);
        else if (MATCH("a ring of *(*)*")) infer (res1, ring);
        else if (MATCH("a wand of *(*)*")) infer (res1, wand);
        else if (MATCH("a staff of *(*)*")) infer (res1, wand);
        else if (MATCH("a scroll of *")) infer (res1, Scroll);
        else if (MATCH("a potion of *(*)*")) infer (res1, potion);
        else if (MATCH("a +*")) ;
        else if (MATCH("an +*")) ;
        else if (MATCH("a -*")) ;
        else if (MATCH("an -*")) ;
        else unknown = true;

        break;

      case 'b':

        if (MATCH("bolt bounces*")) infer ("lightning", wand);
        else if (MATCH("bolt hits*")) infer ("lightning", wand);
        else if (MATCH("bolt misses*")) infer ("lightning", wand);
        else if (MATCH("bummer, this food tastes awful*")) ;
        else if (MATCH("bummer!  you've hit the ground*")) floating = false;
        else if (MATCH("bite has no effect*")) ;
        else unknown = true;

        break;

      case 'c':

        if (MATCH("call it*")) {
	  echoit = false;
	}
        else if (MATCH("call what*")) { echoit = false; }
        else unknown = true;

        break;

      case 'd':

        if (MATCH("defeated the *")) { echoit = false; killed(res1); }
        else if (MATCH("defeated it*")) { echoit = false; killed("it"); }
        else if (MATCH("defeated *")) { echoit = false; killed(res1); }
        else if (MATCH("drop what*")) echoit = false;
        else if (MATCH("destroyed *"))
          { darkturns = 0; darkdir = NONE; targetmonster = 0; echoit = false; }
        else if (MATCH("dropped a scroll * scare monster")) {
          set (STUFF | SCAREM);
          diddrop = true;
        }
        else if (MATCH("dropped *")) {
          set (STUFF | USELESS);
          diddrop = true;
        }
        else unknown = true;

        break;

      case 'e':

        if (MATCH("eat what*")) echoit = false;
        else if (MATCH("everything looks so boring now*")) cosmic = false;
        else unknown = true;

        break;

      case 'f':

        if (MATCH("flame *")) ;
        else if (MATCH("far out!  everything is all cosmic again*")) blinded = false;
        else unknown = true;

        break;

      case 'g':

        if (MATCH("getting hungry*")) echoit = false;
        else if (MATCH("getting the munchies*")) echoit = false;
        else unknown = true;

        break;

      case 'h':

        if (MATCH("hey, this tastes great*")) infer ("restore strength", potion);
        else if (MATCH("huh? what? who?*")) ;
        else if (MATCH("heavy!  that's a nasty critter!*")) ;
        else unknown = true;

        break;

      case 'i':

        if (MATCH("it hit*")) { washit ("it"); echoit = false; }
        else if (MATCH("it misses*"))  { wasmissed ("it"); echoit = false; }
        else if (MATCH("it appears confused*")) ;
        else if (MATCH("ice *")) ;
        else if (MATCH("identify what*")) echoit = false;
        else if (MATCH("illegal command*")) echoit = false;
        else if (MATCH("i see no way*"))
          { unset (STAIRS); findstairs (atrow, atcol); }
        else if (MATCH("it appears to be cursed*")) curseditem ();
        else if (MATCH("it make*")) ;
        else unknown = true;

        break;

      case 'j':
      case 'k':
        unknown = true;
        break;

      case 'l':

        if (MATCH("left or*")) echoit = false;
        else unknown = true;

        break;

      case 'm':

        if (MATCH("missile vanishes*")) infer ("magic missile", wand);
        else if (MATCH("missle vanishes*")) infer ("magic missile", wand);
        else if (MATCH("my, that was a yummy *")) ;
        else if (MATCH("moved onto *")) set (STUFF);

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("multi-colored lines swirl around you, then fade*")) nametrap (BEARTRP,HERE);

        /* :ANT: */

        else unknown = true;

        break;

      case 'n':

        if (MATCH("nothing happens*")) {
          remember (lastwand, WORTHLESS);
        }
        else if (MATCH("no more *")) ;
        else if (MATCH("nothing appropriate*")) sendnow ("%c;",ESC);
        else if (MATCH("no room*")) ;
        else if (MATCH("not wearing armor*")) ;
        else unknown = true;

        break;

      case 'o':

        if (MATCH("oh no! an arrow shot *"))
          { arrowshot = true; nametrap(ARROW,HERE); }
        else if (MATCH("oh, now this scroll has a map *"))
          { infer ("magic mapping", Scroll); didreadmap = Level; }
        else if (MATCH("oh, bummer!  everything is dark!  help!*"))
          { infer ("blindness", potion); blinded = true; }
        else if (MATCH("oh, wow!  everything seems so cosmic!*"))
          { infer ("hallucination", potion); cosmic = true; }
        else if (MATCH("oh, wow!  you're floating in the air!*"))
          { infer ("levitation", potion); floating = true; }
        else if (MATCH("oh, wow, that tasted good*")) ;
        else unknown = true;

        break;

      case 'p':

        if (MATCH("please spec*")) echoit = false;
        else if (MATCH("put on what*")) echoit = false;
        else unknown = true;

        break;

      case 'q':

        if (MATCH("quaff what*")) echoit = false;
        else unknown = true;

        break;

      case 'r':

        if (MATCH("range is 'a' to '*'*")) {
          echoit = false;

          if (*res1-'a'+1 != invcount) {
            dwait (D_INFORM, __func__, "Range check failed");
            usesynch = false;
          }
        }
        else if (MATCH("read what*")) echoit = false;
        else if (MATCH("rogue version *")) echoit = false;
        else unknown = true;

        break;

      case 's':

        if (MATCH("she stole *")) {
          usesynch = false;
        }
        else if (MATCH("sting has no effect*")) ;
        else unknown = true;

        break;

      case 't':

        if (MATCH("throw what*")) echoit = false;
        else if (MATCH("the * bounces*")) ;
        else if (MATCH("the bolt *")) ;
        else if (MATCH("the flame *")) ;
        else if (MATCH("the ice hits*")) ;
        else if (MATCH("the ice misses*")) ;
        else if (MATCH("the ice whizzes by you*")) wasmissed ("ice monster");
        else if (MATCH("the * hits it*")) {echoit = false; mshit ("it");}
        else if (MATCH("the * misses it*")) {echoit = false; msmiss ("it");}
        else if (MATCH("the * hits the *")) {echoit = false; mshit (res2);}
        else if (MATCH("the * misses the *")) {echoit = false; msmiss (res2);}
        else if (MATCH("the * hit*")) { washit (res1); gushed = false; echoit = false; }
        else if (MATCH("the * misses*")) { wasmissed (res1); echoit = false; }
        else if (MATCH("the * appears confused*")) ;
        else if (MATCH("the rust vanishes instantly*"))
          { if (gushed) { gushed = false; nametrap (WATERAP, HERE); } }
        else if (MATCH("the room is lit*")) { setnewgoal (); infer ("light", wand); }
        else if (MATCH("the corridor glows*")) { infer ("light", wand); }
        else if (MATCH("the * has confused you*")) confused = true;
        else if (MATCH("this scroll is an identify scroll scroll*"))
          { readident ("identify scroll"); }
        else if (MATCH("this scroll is an * scroll*")) {
          if (stlmatch (res1, "identify")) {
            readident (res1);
          }
        }
        else if (MATCH("that's not a valid item*")) ;
        else if (MATCH("the veil of darkness lifts*")) blinded = false;
        else if (MATCH("the scroll turns to dust*"))
          { deletestuff (atrow, atcol); unset(SCAREM | STUFF); droppedscare--; }
        else if (MATCH("this potion tastes * dull*")) infer ("thirst quenching", potion);
        else if (MATCH("this potion tastes pretty*")) infer ("thirst quenching", potion);
        else if (MATCH("this potion tastes like * juice*"))
          { infer ("see invisible", potion); if (version == RV36A) sendnow ("%c", ESC); }
        else if (MATCH("this scroll seems to be blank*")) infer ("blank paper", Scroll);
        else if (MATCH("the * bounces*")) ;
        else if (MATCH("the * vanishes as it hits the ground*"))
          { darkturns = 0; darkdir = NONE; targetmonster = 0; echoit = false; }
        else if (MATCH("there is something there already*")) {
          set(STUFF);
          usesynch = false;
        }
        else if (MATCH("there is something here*")) {
          set(STUFF);
          usesynch = false;
        }
        else if (MATCH("the munchies are interfering*")) ;
        else if (MATCH("the monsters around you freeze*")) holdmonsters ();
        else if (MATCH("the monster freezes*")) holdmonsters ();
        else if (MATCH("that's inedible*")) ;

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("time now seems to be going slower*")) nametrap (BEARTRP,HERE);
        else if (MATCH("the light in here suddenly seems*")) nametrap (BEARTRP,HERE);

        /* :ANT: */

        else unknown = true;

        break;

      case 'u':
      case 'v':

        if (MATCH("version *")) echoit = false;
        else unknown = true;

        break;

      case 'w':

        if (MATCH("what do you want*")) echoit = false;
        else if (MATCH("wield wha*")) {
          echoit = false;
          remember (lastdrop, UNCURSED);
          cursedweapon = false;
        }
        else if (MATCH("wielding a*")) ;
        else if (MATCH("wear what*")) echoit = false;
        else if (MATCH("what monster*")) echoit = false;
        else if (MATCH("wait, what's going*")) {infer("confusion", potion); confused = true;}
        else if (MATCH("wait*that's a *")) ;
        else if (MATCH("what a*feeling*")) { infer("confusion", potion); confused = true; }
        else if (MATCH("what a*piece of paper*")) infer ("blank paper", Scroll);
        else if (MATCH("welcome to level *")) ;
        else if (MATCH("was wearing*")) ;
        else if (MATCH("what bulging muscles*")) infer ("gain strength", potion);
        else if (MATCH("wearing *")) ;
        else unknown = true;

        break;

      case 'x':
        unknown = true;
        break;

      case 'y':

        if (MATCH("you hit*")) { echoit = false; didhit (); }
        else if (MATCH("you miss*")) { echoit = false; didmiss (); }
        else if (MATCH("you are starting to feel weak*")) echoit = false;
        else if (MATCH("you are weak from hunger*")) {echoit = false; eat();}
        else if (MATCH("you are being held*")) beingheld=30;
        else if (MATCH("you can move again*")) echoit = false;
        else if (MATCH("you are still stuck *")) nametrap (BEARTRP,HERE);
        else if (MATCH("you can't move*")) echoit = false;
        else if (MATCH("you can't carry anything else*"))
          { echoit = false; set (STUFF); maxobj=objcount; }
        else if (MATCH("you can*")) curseditem ();
        else if (MATCH("you begin to feel better*")) infer ("healing", potion);
        else if (MATCH("you begin to feel much better*")) infer("extra healing", potion);
        else if (MATCH("you begin to sense the presence of monsters*"))
          { infer("monster detection", potion); }
        else if (MATCH("you feel a strange sense of loss*")) infer("hold monster", Scroll);
        else if (MATCH("you feel a wrenching sensation in your gut*")) ;
        else if (MATCH("you feel stronger, now*")) infer ("gain strength", potion);
        else if (MATCH("you feel very sick now*")) infer ("poison", potion);
        else if (MATCH("you feel momentarily sick*")) infer ("poison", potion);
        else if (MATCH("you suddenly feel much more skillful*"))
          { infer("raise level", potion); }
        else if (MATCH("your nose tingles*")) infer ("food detection", Scroll);
        else if (MATCH("you start to float in the air*"))
          { infer ("levitation", potion); floating = true; }
        else if (MATCH("you're floating off the ground!*")) floating = true;
        else if (MATCH("you float gently to the ground*")) floating = false;
        else if (MATCH("you feel yourself moving much faster*"))
          { infer ("haste self", potion); hasted = true; }
        else if (MATCH("you feel yourself slowing down*"))
          { hasted = false; doublehasted = false; }
        else if (MATCH("you faint from exhaustion*"))
          { if (version < RV52A) doublehasted = true; else hasted = false; }
        else if (MATCH("you feel less confused now*")) confused = false;
        else if (MATCH("you feel less trip*")) confused = false;
        else if (MATCH("your * vanishes as it hits the ground*"))
          { darkturns = 0; darkdir = NONE; echoit = false; }
        else if (MATCH("your hands begin to glow *"))
          { infer ("monster confusion", Scroll); redhands = true; }
        else if (MATCH("your hands stop glowing *")) redhands = false;

        else if (MATCH("you feel as if somebody is watching over you*") ||
                 MATCH("you feel in touch with the universal onenes*")) {
          infer ("remove curse", Scroll);
          forget (currentarmor, CURSED);
          remember (currentarmor, UNCURSED);
          cursedarmor = false;
          forget (currentweapon, CURSED);
          remember (currentweapon, UNCURSED);
          cursedweapon = false;
          newarmor = newweapon = true;
        }

        else if (MATCH("your armor weakens*")) {
          inven[currentarmor].phit--;

          if (gushed) { gushed = false; nametrap (WATERAP,HERE); }
        }

        else if (MATCH("your scalp itches")) infer ("protect armor", Scroll);
        else if (MATCH("your armor is covered by a shimmering * shield*")) {
          infer ("protect armor", Scroll);
          forget (currentarmor, CURSED);
          remember (currentarmor, UNCURSED);
          cursedarmor = false;
          protected = true;
          remember (currentarmor, PROTECTED);
        }

        else if (MATCH("your arms tingle")) infer ("enchant armor", Scroll);
        else if (MATCH("your armor glows * for a moment*")) {
          infer ("enchant armor", Scroll);
          cursedarmor = false;
          newarmor = true;
          inven[currentarmor].phit++;

          if (itemis(currentarmor, UNCURSED))
            remember (currentarmor, ENCHANTED);

          forget (currentarmor, CURSED);
          remember (currentarmor, UNCURSED);
        }
        else if (MATCH("your hands tingle")) infer ("enchant weapon", Scroll);
        else if (MATCH("your * glows * for a moment*")) {
          infer ("enchant weapon", Scroll);
          cursedweapon = false;
          newweapon = true;

          if (itemis (currentweapon, UNCURSED))
            remember (currentweapon, ENCHANTED);

          forget (currentweapon, CURSED);
          remember (currentweapon, UNCURSED);
        }
        else if (MATCH("you hear a high pitched humming noise*"))
          { infer ("aggravate monsters", Scroll); wakemonster (9); aggravated = true; }
        else if (MATCH("you hear maniacal laughter*")) infer ("scare monster", Scroll);
        else if (MATCH("you hear a faint cry*")) infer ("create monster", Scroll);
        else if (MATCH("you fall asleep*")) infer ("sleep", Scroll);
        else if (MATCH("you have been granted the boon of genocide*"))
          { infer ("genocide", Scroll); echoit = false; rampage (); }
        else if (MATCH("you have a tingling feeling*")) infer ("drain life", wand);
        else if (MATCH("you are too weak to use it*")) infer ("drain life", wand);
        else if (MATCH("you begin to feel greedy*")) infer ("gold detection", potion);
        else if (MATCH("you feel a pull downward*")) infer ("gold detection", potion);
        else if (MATCH("you begin to feel a pull downward*"))
          { infer ("gold detection", potion); }
        else if (MATCH("you are caught *")) nametrap (BEARTRP,HERE);
        else if (MATCH("your purse feels lighter*")) ;
        else if (MATCH("you suddenly feel weaker*")) ;
        else if (MATCH("you must identify something*")) ;
        else if (MATCH("you have a * feeling for a moment, then it passes*")) infer ("monster detection", potion);
        else if (MATCH("you have a * feeling for a moment*")) infer ("monster detection", potion);
        else if (MATCH("you daydream of * for a moment, then it passes*")) infer ("magic detection", potion);
        else if (MATCH("you feel deeply moved*")) infer ("teleportation", Scroll);

        else if (MATCH("you are transfixed*")) ;
        else if (MATCH("you are frozen*")) washit ("ice monster");
        else if (MATCH("you faint*")) {echoit = false; if (version<RV36B) eat();}
        else if (MATCH("you freak out*")) echoit = false;
        else if (MATCH("you fell into a trap!*")) ;
        else if (MATCH("yum*")) echoit = false;
        else if (MATCH("yuk*")) echoit = false;
        else if (MATCH("you sense the presence of magic*")) { infer ("magic detection", potion); echoit = false; }

        /* :ANT: let's tag this as a BEARTRP for now */
        else if (MATCH("you are suddenly in a parallel dimension*")) nametrap (BEARTRP, HERE);
        else if (MATCH("you feel a sting in the side of your neck*")) nametrap (BEARTRP, HERE);
        else if (MATCH("you feel time speed up suddenly*")) nametrap (BEARTRP, HERE);
        else if (MATCH("you suddenly feel very thirsty*")) nametrap (BEARTRP, HERE);
        else if (MATCH("yo* pack turns *")) nametrap (BEARTRP, HERE);

        /* :ANT: */

        /* :ANT: logic error indicator */
        else if (MATCH("you are already wearing some*"))
          dwait (D_ERROR, __func__, "Logic error: %s", mess);

        /* :ANT: */

        else unknown = true;

        break;

      case 'z':

        if (MATCH("zap with what*")) echoit = false;
        else unknown = true;

        break;

      default:

        if (MATCH( "* gold pieces*")) { echoit = false; countgold (res1); }
        else if (MATCH("(mctesq was here)*")) echoit = false;
        else if (MATCH("'*'*: *")) { echoit = false; mapcharacter (*res1, res3); }
        else if (*mess == '+' || *mess == '-' || ISDIGIT (*mess)) echoit = false;
        else if (MATCH("'*' is not a valid item*")) echoit = false;
        else unknown = true;

        break;
    }

  /* Log unknown or troublesome messages */
  if ((morecount > 150) && (morecount < 200)) {
    dwait(D_WARNING, __func__, "More Loop ->%s<-.", mess);
  }
  else if (morecount >= 200) {
    dwait(D_FATAL, __func__, "More Loop Exit ->%s<-.", mess);
  }
  else if (unknown)
    dwait (D_WARNING, __func__, "Unknown message: %s", mess);

  /* Send it to dwait; if dwait doesnt print it (and echo is on) echo it */
  if (echoit & !dwait (D_MESSAGE, __func__, "%s", mess))
    saynow ("%s", mess);
}

/*
 * smatch: Given a data string and a pattern containing one or
 * more embedded stars (*) (which match any number of characters)
 * return true if the match succeeds, and set res[i] to the
 * characters matched by the 'i'th *.
 */

static int
smatch (char *dat, char *pat, char **res)
{
  char *star = 0, *starend, *resp;
  int nres = 0;

  while (1) {
    if (*pat == '*') {
      star = ++pat;			     /* Pattern after * */
      starend = dat;			     /* Data after * match */
      resp = res[nres++];		     /* Result string */
      *resp = '\0';			     /* Initially null */
    }
    else if (*dat == *pat) {		     /* Characters match */
      if (*pat == '\0')			     /* Pattern matches */
        return (1);

      pat++;				     /* Try next position */
      dat++;
    }
    else {
      if (*dat == '\0')			     /* Pattern fails - no more */
        return (0);			     /* data */

      if (star == 0)			     /* Pattern fails - no * to */
        return (0);			     /* adjust */

      pat = star;			     /* Restart pattern after * */
      *resp++ = *starend;		     /* Copy character to result */
      *resp = '\0';			     /* null terminate */
      dat = ++starend;			     /* Rescan after copied char */
    }
  }
}

/*
 * readident: we have read an identify scroll.
 */

static void
readident (char *name)
{
  int obj; char id = '*';	/* Default is "* for list" */
  stuff item_type = none;
  char lookup_name[NAMSIZ + 1];	/* +1 for paranoia */

  if (!replaying && version < RV53A &&
      (nextid < LETTER (0) || nextid > LETTER (invcount))) {
    dwait (D_FATAL, __func__, "nextid: %d afterid: %d invcount: %d",
           nextid, afterid, invcount);
  }

  infer (name, Scroll);		/* Record what kind of scroll this is */

  at (0,0);
  clrtoeol ();
  memset (&(screen[0]), ' ', sizeof(screen[0]));
  at (row, col);
  refresh ();

  if (version < RV53A) {	/* Rogue 3.6, Rogue 5.2 */
    deleteinv (OBJECT (afterid));	/* Assume object gone */
    sendnow (" %c", nextid);		/* Identify it */

    if (!replaying)   /* FIXME: without removing the rogo_send call during
                       * replay, replay will core dump - NYM
                       */
    {
      rogo_send ("I%c", afterid);		/* Generate a message about it */
    }

    knowident = true;	/* Set variables */
    identifying = true;	/* Set variables */
  }
  else {			/* Rogue 5.3 */
    if (streq (name, "identify ring, wand or staff")) {
      if ((obj = unknown (ring)) != NONE) {
        id = LETTER (obj);
        item_type = ring;
      }
      else if ((obj = unknown (wand)) != NONE) {
        id = LETTER (obj);
        item_type = wand;
      }
      else if ((obj = have (ring)) != NONE) {
        id = LETTER (obj);
        item_type = ring;
      }
      else if ((obj = have (wand)) != NONE) {
        id = LETTER (obj);
        item_type = wand;
      }
    }
    else if (streq (name, "identify potion")) {
      if ((obj = unknown (potion)) != NONE || (obj = have (potion)) != NONE) {
        id = LETTER (obj);
        item_type = potion;
      }
    }
    else if (streq (name, "identify armor")) {
      if ((obj = unknown (armor)) != NONE || (obj = have (armor)) != NONE)
        id = LETTER (obj);
    }
    else if (streq (name, "identify weapon")) {
      if ((obj = unknown (hitter)) != NONE ||
          (obj = unknown (thrower)) != NONE ||
          (obj = unknown (missile)) != NONE ||
          (obj = have (hitter)) != NONE ||
          (obj = have (thrower)) != NONE ||
          (obj = have (missile)) != NONE)
        id = LETTER (obj);
    }
    else if (streq (name, "identify scroll") || streq (name, "identify")) {
      if ((obj = unknown (Scroll)) != NONE || (obj = have (Scroll)) != NONE) {
        id = LETTER (obj);
        item_type = Scroll;
      }
    }

    if ((id != '*') &&
        (item_type == ring || item_type == wand || item_type == potion || item_type == Scroll)) {
     char *fakename;

      memset (lastname, 0, sizeof(lastname));
      memset (lookup_name, 0, sizeof(lookup_name));
      strlcpy (lookup_name, inven[obj].str, sizeof(lookup_name));
      fakename = findentry_getfakename (lookup_name, item_type);
      strlcpy (lastname, fakename, sizeof(lookup_name));
    }

    waitfor ("not a valid item");
    sendnow (" %c;", id);		    /* Pick an object to identify */
    if (id == '*')
      memset (lastname, '\0', sizeof(lastname));
    usesynch = false; justreadid = true;    /* Must reset inventory */
  }

  newring = newweapon = true; afterid = nextid = '\0';
}

/*
 * rampage: read a scroll of genocide.
 */

static void
rampage (void)
{
  char monc;

  /* Check the next monster in the list, we may not fear him */
  while ((monc = *genocide)) {
    /* Do not waste genocide on stalkers if we have the right ring */
    if ((streq (monname (monc), "invisible stalker") ||
         streq (monname (monc), "phantom")) &&
        havenamed (ring, "see invisible") != NONE)
      { genocide++; }

    /* Do not waste genocide on rusties if we have the right ring */
    else if ((streq (monname (monc), "rust monster") ||
              streq (monname (monc), "aquator")) &&
             havenamed (ring, "maintain armor") != NONE)
      { genocide++; }

    /* No fancy magic for this monster, use the genocide scroll */
    else break;
  }

  /* If we found a monster, send his character, else send ESC */
  if (monc) {
    size_t len;	/* length of genocided */

    saynow ("About to rampage against %s", monname (monc));
    sendnow (" %c;", monc);	/* Send the monster */

    /* Add to the list of 'gone' monsters */
    len = strlen(genocided);
    if (len+1 < sizeof(genocided)) { /* paranoia */
      genocided[len] = monc;
      genocided[len+1] = '\0'; /* paranoia */
      genocide++;
    } else {
      dwait (D_ERROR, __func__, "genocide table is full");
      sendnow (" %c;", ESC);	/* Cancel the command */
    }
  }
  else {
    dwait (D_ERROR, __func__, "Out of monsters to genocide");
    sendnow (" %c;", ESC);	/* Cancel the command */
  }
}

/*
 * curseditem: the last object we tried to drop (unwield, etc.)  was cursed.
 *
 * Note that cursed rings are not a problem since we only put on
 * Good rings we have identified, so don't bother marking rings.
 */

static void
curseditem (void)
{
  usesynch = false;    /* Force a reset inventory */

  clearsendqueue();
  sendnow ("%c", ESC);

  /* lastdrop is index of last item we tried to use which could be cursed */
  if (lastdrop != NONE && lastdrop < invcount) {
    remember (lastdrop, CURSED);

    /* Is our armor cursed? */
    if (inven[lastdrop].type == armor)
      { currentarmor = lastdrop; cursedarmor = true; return; }

    /* Is it our weapon (may be wielding a hitter or a bogus magic arrow)? */
    else if (inven[lastdrop].type==hitter || inven[lastdrop].type==missile)
      { currentweapon = lastdrop; cursedweapon = true; return; }
  }

  /* Don't know what was cursed, so assume the worst */
  cursedarmor = true;
  cursedweapon = true;

}

/*
 * First copy the title of the last scroll into the appropriate slot,
 * then find the real name of the object by looking through the data
 * base, and then zap that name into all of the same objects
 */

void
infer (char *objname, stuff item_type)
{
  int i;

  if (*lastname && *objname && !stlmatch (objname, lastname)) {
    infername (lastname, objname, item_type);
    for (i=0; i<MAXINV; i++) {
      if ((inven[i].count > 0) &&
           streq (inven[i].str, lastname) && inven[i].type == item_type) {
        memset (inven[i].str, '\0', NAMSIZ); /* sizeof(space[x]) is NAMSIZ + 1 chars */
        strlcpy (inven[i].str, objname, NAMSIZ); /* sizeof(space[x]) is NAMSIZ + 1 chars */
        remember (i, KNOWN);
      }
    }
  }
}

/*
 * Killed: called whenever we defeat a monster.
 */

void
killed (char *monster)
{
  int m = 0, mh = 0;

  /* Find out what we really killed */
  if (!cosmic && !blinded && targetmonster>0 && streq (monster, "it"))
    { monster = monname (targetmonster); }

  if ((mh = getmonhist (monster, 0)) != NONE)
    { monster = monhist[mh].m_name; m = monsternum (monster); }

  /* Tell the user what we killed */
  dwait (D_BATTLE | D_MONSTER, __func__, "Killed: %s", monster);

  /* If cheating against Rogue 3.6, check out our arrow */
  if (version < RV52A && cheat) {
    if (usingarrow && hitstokill > 1 && !beingstalked && goodarrow < R-4) {
      saynow ("Oops, bad arrow...");
      newweapon = badarrow = true; remember (currentweapon, WORTHLESS);
    }
    else if (usingarrow) goodarrow++;
  }

  /* Echo the number arrows we pumped into him */
  if (mh >=0 && mhit+mmiss > 0 && mtarget == mh)
    dwait (D_BATTLE | D_MONSTER, __func__, "%d out of %d missiles hit: %s",
           mhit, mhit+mmiss, monster);

  /* If we killed it by hacking, add the result to long term memory */
  if (hitstokill > 0 && mh != NONE)
    addstat (&monhist[mh].htokill, hitstokill);

  /* If we killed it with arrows, add that fact to long term memory */
  if (mhit > 0 && mh != NONE)
    addstat (&monhist[mh].atokill, mhit);

  /* Stop shooting arrows if we killed the right monster */
  if (targetmonster == (m+'A'-1))
    { darkturns = 0; darkdir = NONE; targetmonster = 0; }

  goalr = goalc = NONE;			/* Clear old movement goal */
  monkilled[m]++; totalkilled++;	/* Bump kill count */
  hitstokill = mhit = mmiss = 0;	/* Clear indiviual monster stats */
  mtarget = NONE;			/* Clear target */
  beingheld = false;			/* Clear flags */
  cancelled = false;			/* Clear flags */

  /* If we killed an invisible, assume no more invisible around */
  if (!cosmic && !blinded &&
      (streq (monster, "invisible stalker") || streq (monster, "phantom")))
    beingstalked = 0;
}

/*
 * washit: Record being hit by a monster.
 */

static void
washit (char *monster)
{
  int mh = 0, m = 0;

  /* Find out what really hit us */
  if ((mh = getmonhist (monster, 1)) != NONE)
    { monster = monhist[mh].m_name; m = monsternum (monster); }

  dwait (D_MONSTER, __func__, "Was hit by: %s", monster);

  timeshit++;			/* Bump global count */

  if (m>0) wakemonster(-m);	/* Wake him up */

  terpbot ();			/* Hit points changed, read bottom */

  /* Add data about the event to long term memory */
  if (mh != NONE) {
    addprob (&monhist[mh].theyhit, SUCCESS);
    addstat (&monhist[mh].damage, lastdamage);
    analyzeltm ();
  }
}

/*
 * wasmissed: Record being missed by a monster.
 */

static void
wasmissed (char *monster)
{
  int mh = 0, m = 0;

  /* Find out what really missed us */
  if ((mh = getmonhist (monster, 1)) != NONE)
    { monster = monhist[mh].m_name; m = monsternum (monster); }

  dwait (D_MONSTER, __func__, "Was missed by: %s", monster);

  timesmissed++;		/* Bump global count */

  if (m>0) wakemonster(-m);	/* Wake him up */

  /* Add data to long term memory */
  if (mh != NONE) {
    addprob (&monhist[mh].theyhit, FAILURE);
    analyzeltm ();
  }
}

/*
 * didhit: Record hitting a monster.
 */

static void
didhit (void)
{
  int m = 0;

  /* Record our hit */
  if (!cosmic) m = lastmonster;

  hits++; hitstokill++;
  addprob (&monhist[monindex[m]].wehit, SUCCESS);

  if (wielding (wand))
    { inven[currentweapon].charges--; newweapon = true; }
}

/*
 * didmiss: Record missing a monster.
 */

static void
didmiss (void)
{
  int m = 0;

  /* Record our miss */
  if (!cosmic) m = lastmonster;

  misses++;
  addprob (&monhist[monindex[m]].wehit, FAILURE);

  if (usingarrow && goodarrow < R-4)
    { newweapon = badarrow = true; remember (currentweapon, WORTHLESS); }
}

/*
 * mshit: Record hitting a monster with a missile.
 */

static void
mshit (char *monster)
{
  int mh;

  /* Arching in a dark room? */
  if (!cosmic && !blinded && targetmonster > 0 && streq (monster, "it"))
    monster = monname (targetmonster);

  /* Add data about the event to long term memory */
  if ((mh = getmonhist (monster, 0)) < 0) return;

  {
    addprob (&monhist[monindex[mh]].arrowhit, SUCCESS);

    if (mh == mtarget) { mhit++; }
    else { mhit=1; mmiss = 0; mtarget=mh; }
  }
}

/*
 * msmiss: Record missing a monster with a missile.
 */

static void
msmiss (char *monster)
{
  int mh;

  /* Arching in a dark room? */
  if (!cosmic && !blinded && targetmonster > 0 && streq (monster, "it"))
    monster = monname (targetmonster);

  /* Add data about the event to long term memory */
  if ((mh = getmonhist (monster, 0)) < 0) return;

  {
    addprob (&monhist[monindex[mh]].arrowhit, FAILURE);

    if (mh == mtarget) { mmiss++; }
    else { mmiss=1; mhit=0; mtarget=mh; }
  }
}

/*
 * Countgold: called whenever msg contains a message about the number
 *            of gold pieces we just picked up. This routine keeps
 *            statistics about the amount of gold picked up.
 */

static void
countgold (char *amount)
{
  int pot;

  if ((pot = atoi (amount)) > 0)
    { sumgold += pot; sumsqgold += pot*pot; numgold ++; }
}

/*
 * Summary: print a summary of the game.
 */

void
summary (FILE *f, char sep)
{
  int m;
  char s[BIGBUF + 1], s2[BIGBUF + 1];

  memset (s, 0, sizeof(s)); /* paranoia */
  snprintf (s, MU_BUF, "Monsters killed:%c%c", sep, sep);

  for (m=0; m<=26; m++) {
    if (monkilled[m] > 0) {
      memset (s2, 0, sizeof(s2)); /* paranoia */
      snprintf (s2, sizeof(s2), "\t%d %.*s%.*s%c",
	        monkilled[m], MU_BUF, monname (m+'A'-1),
                MU_BUF, plural (monkilled[m]), sep);
      strlcat (s, s2, sizeof(s));
    }
  }

  memset (s2, 0, sizeof(s2)); /* paranoia */
  snprintf (s2, sizeof(s2), "%cTotal: %d%c%c", sep, totalkilled, sep, sep);
  strlcat (s, s2, sizeof(s));

  memset (s2, 0, sizeof(s2)); /* paranoia */
  snprintf (s2, sizeof(s2), "Hit %d out of %d times, was hit %d out of %d times.%c",
            hits, misses+hits, timeshit, timesmissed+timeshit, sep);
  strlcat (s, s2, sizeof(s));

  if (numgold > 0) {
    memset (s2, 0, sizeof(s2)); /* paranoia */
    snprintf (s2, sizeof(s2), "Gold %d total, %d pots, %d average.%c",
              sumgold, numgold, (sumgold*10+5) / (numgold*10), sep);
    strlcat (s, s2, sizeof(s));
  }

  if (f == NULL)
    addstr (s);
  else
    fprintf (f, "%s", s);
}

/*
 * versiondep: Set version dependent variables.
 */

void
versiondep (void)
{
  if (version >= RV53A)		genocide = "DMJGU";
  else if (version >= RV52A)	genocide = "UDVPX";
  else				genocide = "UXDPW";

  analyzeltm ();
}

/*
 * getmonhist: Retrieve the index in the history array of a monster,
 * taking our status into account.  This code is responsible for determining
 * when we are being stalked by an invisible monster.
 */

static int
getmonhist (char *monster, int hitormiss)
{
  if (cosmic || blinded)
    { return (findmonster ("it")); }
  else {
    if (streq (monster, "it") && hitormiss) {
      if (version < RV53A) {
        if (! seemonster ("invisible stalker")) beingstalked=INVHIT;

        return (findmonster ("invisible stalker"));
      }
      else {
        if (! seemonster ("phantom")) beingstalked=INVHIT;

        return (findmonster ("phantom"));
      }
    }
    else {
      if (version < RV52B && streq (monster, "invisible stalker") &&
          ! seemonster (monster))
        beingstalked = INVHIT;

      return (findmonster (monster));
    }
  }
}
