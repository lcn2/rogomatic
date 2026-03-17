/*
 * Rog-O-Matic
 * Automatically exploring the dungeons of doom.
 *
 * Copyright (C) 2008 by Anthony Molinaro
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

# include <sys/types.h>
# include <stdlib.h>
# include <string.h>

# include "install.h"

/* 
 * If RGMDIR environment variable was set, use it.
 * Otherwise, use RGMDIR and LOCKFILE values according to the definition in install.h
 */

const char *
getRgmDir (void)
{
  char *path = getenv("RGMDIR");
  return path != NULL ? path : RGMDIR;
}

const char *
getLockFile (void)
{
  char *path = getenv("RGMDIR");
  if (path == NULL) {
    return LOCKFILE;
  }

  char* buf = calloc(strlen(path) + 9, sizeof(char));
  strcpy(buf, path);
  strcat(buf, "/Rgm.Lock");

  return buf;
}
