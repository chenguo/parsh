/* Header for file list and dependency. 
   Copyright (C) 2010 Chen Guo

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Written by Chen Guo, chenguo4@ucla.edu.
   Under advisement of Paul Eggert.  */

#ifndef FILE_H
#define FILE_H

#include <stdbool.h>

#include "dgraph.h"

/* Access type. */
enum
  {
    READ_ACCESS,
    WRITE_ACCESS,
    RW_ACCESS
  };

/* File being accessed by one or more commands. */
struct f_file
{
  /* TDOO: inode */
  char *path;                    /* Absolute path to the file. */
  struct f_access *accessors;    /* Commands accessing file. */
  struct f_file *next;           /* Next file. */
};

/* File accessor. */
struct f_access
{
  int access;                    /* Access type. */
  struct dg_node *node;          /* Graph node of command. */
};

#endif
