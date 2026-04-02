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


#if !defined(INCLUDE_TERMTOKENS_H)
#define INCLUDE_TERMTOKENS_H


/*
 * termtokens.h:
 *
 * Various tokens used by the screen reading package.
 */

# define BS_TOK ctrl('H')       /* 0x08 - Move left by one */
# define CE_TOK -2              /* 0xfe - Clear screen */
# define CL_TOK ctrl('L')       /* 0x0c - Clear screen and go to home 0,0 */
# define CM_TOK -3              /* 0xfd - Move to position number1, number2 */
# define CR_TOK ctrl('M')       /* 0x0d - Move to column 0 */
//# define DO_TOK -4            /* 0xfc - UNUSED */
# define ER_TOK -5              /* 0xfb - error token - no effect */
# define LF_TOK ctrl('J')       /* 0x0a - Line feed, move down one row */
# define ND_TOK -6              /* 0xfa - Move down number1 rows */
# define SE_TOK -7              /* 0xf9 - End standout mode */
# define SO_TOK -8              /* 0xf8 - Start standout mode */
# define TA_TOK ctrl('I')       /* 0x09 - Tab to next hw stop */
# define UP_TOK -9              /* 0xf7 - Move up one row */
# define HM_TOK -10             /* 0xf6 - Go to home 0,0 */
# define CH_TOK -11             /* 0xf5 - Change scroll region to number1 - number2 */
# define NU_TOK -12             /* 0xf4 - Move up number1 rows */
# define NR_TOK -13             /* 0xf3 - Move right number1 cols */
# define NL_TOK -14             /* 0xf2 - Move left number1 cols */
# define SC_TOK -15             /* 0xf1 - Save cursor */
# define RC_TOK -16             /* 0xf0 - Restore cursor */
# define SR_TOK -17             /* 0xef - Scroll reverse */
# define CB_TOK -18             /* 0xee - Clear to beginning of line */


#endif
