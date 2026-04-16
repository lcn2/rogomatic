/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2026  Landon Curt Noll
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


#if !defined(INCLUDE_CONFIG_H)
#define INCLUDE_CONFIG_H


/*
 * global declarations
 */
extern char rgmdir[MU_BUF + 1];		/* rogomatic directory - may include UTC date and time sub-dir, +1 for paranoia */
extern char lock_path[TY_BUF + 1];	/* rogomatic lock file path, +1 for paranoia */
extern unsigned int dnum;		/* rogue dungeon number */


#endif
