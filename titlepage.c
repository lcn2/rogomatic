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
 * titlepage.c:
 *
 * This file contains the functions which display the animated copyright
 * notice on the screen.  A general movie facility is used to animate
 * the screen.
 */

# include <stdio.h>
# include <stdlib.h>
# include <curses.h>
# include <time.h>
# include <limits.h>

# include "types.h"
# include "globals.h"

/* static declarations */

static char *titlepage[]= {
  /* The static part of the display */
  "  @ !@ \"@ #@ K@! @!$@!K@!P@\" @\"$@\"(@\")@\"*@\"/@\"0@\"1@\"6@\"7@\"8@",
  "\"=@\"?@\"C@\"D@\"E@\"I@\"J@\"K@\"L@\"M@\"U@\"V@\"W@\"X@\"]@\"a@\"c@\"e@",
  "\"i@# @#!@#\"@##@#'@#+@#.@#2@#5@#9@#<@#>@#@@#F@#K@#P@#T@#^@#`@#c@#e@#i@$",
  " @$$@$'@$+@$.@$2@$5@$9@$<@$@@$C@$D@$E@$F@$K@$P@$T@$_@$c@$e@$i@% @%$@%'@%",
  "+@%.@%2@%5@%9@%<@%@@%C@%F@%K@%P@%T@%^@%`@%c@%f@%h@& @&%@&(@&)@&*@&/@&0@&",
  "1@&2@&6@&7@&8@&<@&@@&D@&E@&G@&L@&Q@&U@&V@&W@&X@&]@&a@&c@&g@'2@(.@(/@(0@(",
  "1@);C)<o)=p)>y)?r)@i)Ag)Bh)Ct)E()Fc)G))I1)J9)K8)L5)Nb)Oy*'A*(n*)d**r*+e*",
  ",w*.A*/p*0p*1e*2l*3,*5L*6e*7o*8n*9a*:r*;d*=H*>a*?m*@e*Ay*B,*DG*Eu*Fy*HJ*",
  "Ia*Jc*Ko*Lb*Ms*No*On*P,*Ra*Sn*Td*VM*Wi*Xc*Yh*Za*[e*\\l*^M*_a*`u*al*bd*ci",
  "*dn,)@,*@,+@,,@,[D,\\D,]D,^D,_D-(@-+@--@-[D-`D.(@.*@.-@.[D.`D/(@/*@/+@/,",
  "@/-@/[D/`D0)@0*@0+@0,@0[D0\\D0]D0^D0_D2 H2!o2\"n2#o2$r2%a2&b2'l2(e2*m2+e",
  "2,m2-b2.e2/r21o22f24t25h26e28F29i2:g2;h2<t2=e2>r2?'2@s2BG2Cu2Di2El2Fd3 T",
  "3!o3\"t3#a3$l3&w3'i3(n3)n3*e3+r3-a3.g3/a30i31n32s33t35R36o37g38u39e3;33<",
  ".3=63>,3@S3Ae3Bp3Ct3De3Em3Fb3Ge3Hr3J23K63L,3N13O93P83Q24 T4!o4\"t4#a4$l4",
  "&w4'i4(n4)n4*e4+r4-a4.g4/a40i41n42s43t45R46o47g48u49e4;54<.4=24>,4@O4Ac4",
  "Bt4Co4Db4Ee4Fr4H14I04J,4L14M94N84O35 T5!o5\"t5#a5$l5&w5'i5(n5)n5*e5+r5-a",
  "5.g5/a50i51n52s53t55R56o57g58u59e5;55<.5=35>,5@F5Ae5Bb5Cr5Du5Ea5Fr5Gy5I1",
  "5J65K,5M15N95O85P46 c6!h6\"o6#n6$g6%o6& 6'<6(L6)a6*n6+d6,o6-n6. 6/C60u61",
  "r62t63 64N65o66l67l68>69 6:/6;\\6<.6=.6>/6?\\~~~~~~~~~~~~~~~~~~~~~~~~~~~",

  /* The dynamic part of the display */
  "~~~~~~~~~~00/~/1/~.2)~-1\\~,0\\~~~.3>~.4=~.5=~.6=~.7=~.8=~.9>~~~.2>.3=.8",
  ">.9 ~~~.1>.2=.7>.8 ~~~.0>.1=.6>.7 ~~~./>.0=.5>.6 ~~~..>./=.4>.5 ~~~~~~~~",
  "~~~~~.. ./>.4 .4=.5>~./ .0>.5=.6>~.0 .1>.6=.7>~.1 .2>.7=.8>~.2).3>.8=.9>",
  "~.3 .4>.9=.:>~.4 .5>.:=.;>~.5 .6>.;=.<>~.6 .7>.<=.=>~.7 .8>.==.>>~.8 .9>",
  ".>=.?>~.9 .:>.?=.@>~.: .;>.@=.A>~.; .<>.A=.B>~.< .=>.B=.C>~.= .>>.C=.D>~",
  ".> .?>.D=.E>~.? .@>.E=.F>~.@ .A>.F=.G>~.A .B>.G=.H>~.B .C>.H=.I>~.C .D>.",
  "I=.J>~.D .E>.J=.K>~.E .F>.K=.L>~.F .G>.L=.M>~.G .H>.M=.N>~.H .I>.N=.O>~.",
  "I .J>.O=.P>~.J .K>.P=.Q>~.K .L>.Q=.R>~.L .M>.R=.S>~.M .N>.S=.T>~.N .O>.T",
  "=.U>~.O .P>.U=.V>~.P .Q>.V=.W>~.Q .R>.W=.X>~.R .S>.X=.Y>~.S .T>.Y=.Z>~.T",
  " .U>.Z=.[>~.U .V>.[=.\\>~.V .W>.\\=.]>~.W .X>.]=.^>~.X .Y>.^=._>~.Y .Z>.",
  "_=.`>~.Z .[>.`=.a>~.[ .\\>.a=.b>~.\\ .]>.b=.c>~.] .^>.c=.d>~.^ ._>.d=.e>",
  "~._ .`>.e=.f>~.` .a>.f=.g>~.a .b>.g=.h>~.b .c>.h=.i>~.c .d>.i ~.d .e>~.e",
  " .f>~.f .g>~.g .h>~.h ~~~~~.[D,[ ,\\j,\\ -\\D-]d-]D-^D-_D,_ ,^ ,] ,]j,] ",
  "~-[ .ZD/ZD.[ .[L.[ .\\D.]S.]D.^D._D.`D/aD-\\ -\\L-\\ -] -^ -_ -` ~/YD.Z ",
  "/\\D/]D/^D/_D.\\ .] .^ ._ .` 0`D0aD/a ~0XD0YD0ZD/Y /Z /` ~0VD0WD/[ /\\ /",
  "^ /_ 0bD0cD/] 0UD0dD~~~~~~~~~~,0 ~~-1 ~~.2 ~~/1 ~~00 ~~~~~~~~~~~~~~~~~~~",
  "~~.5S~.6i~.7l~.8l~.9y~.;D~.<r~.=a~.>g~.?o~.@n~.B:~.C-~.D)~.E ~~~~~~~~~~~",
  "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
  ".5 .6 .7 .8 .9 .: .; .< .= .> .? .@ .A .B .C .D .E ~~~~~~~~~~~~~~~~~~~~~",
  NULL
};

static void animate (char *movie[]);

# define NEXTCHAR (*cbf?*cbf++:(cbf=1+ *movie++)[-1])

/*
 * animate: Display a movie on the screen.  A movie is a list of strings
 * wherein each character is either a bell command '}', a synchronize
 * command '~', or a triplet <row+32, col+32, char> indicating a character
 * to be placed at a specific place on the screen.
 *
 * Movies run the same speed regardless of baudrate.
 */

static void
animate (char *movie[])
{
  int r, c;
  char *cbf = "";
  struct timespec rqt = { 0, 1e7 };		/* short 0.01 seconds */
  struct timespec rqt2 = { 0, 5e8 };		/* longer 0.5 seconds */

  if (emacs || terse) return;			/* No screen ==> no movie */

  clear ();					/* Clear the screen */

  while (*movie || *cbf) {			/* While more animate commands */
    r = NEXTCHAR;				/* Get command character */

    /* Do NOT Ring the Bell */
    if (r == '}') {
#if 0 /* no beep */
      beep();
      flash();
      refresh ();				/* Write out screen */
#endif
    }

    /* Update the screen and delay until one timestep is gone */
    else if (r == '~') {
      refresh ();				/* Write out screen */
      (void) nanosleep(&rqt, NULL);
    }

    /* Write out a single character */
    else {
      r -= 32;					/* Get screen row */
      c = NEXTCHAR - 32;			/* Get screen col */
      mvaddch (r, c, NEXTCHAR);			/* Write out character */
    }
  }
  (void) nanosleep(&rqt2, NULL);
}

/*
 * halftimeshow: If its time for a halftime show, call movie.  Variable
 * 'nohalf' is true if the user does not want to see a halftime show.
 */

void
halftimeshow (int level)
{
  static int nextshow = 1;

  /* do nothing is halftime shows are disabled */
  if (nohalf) {
    return;
  }

  /* determine if the level is deep enough */
  if (level >= nextshow) {

    /* we are deep enough, display the title page */
    animate (titlepage);

    /* determine the next halftime show level */
    switch (nextshow) {
    case 1:
	nextshow = 12;
	break;
    case 12:
	nextshow = 25;
	break;
    case 25:
	nextshow = 31;
	break;
    case 31:
	nextshow = INT_MAX;
	break;
    default:
	break;
    }
  }
  return;
}
