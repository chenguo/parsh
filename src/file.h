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

#include "command.h"

/* Access type. */
enum
  {
    NO_ACCESS,
    READ_ACCESS,
    WRITE_ACCESS,
    RW_ACCESS
  };

/* File being accessed by one or more commands. */
struct file
{
  /* TDOO: inode */
  int access;                    /* Type of current access. */
  char *path;                    /* Absolute path to the file. */
  struct file_acc *accessors;    /* Commands accessing file. */
  struct file_acc *acc_tail;     /* Last accessor in list. */
  struct file *next;             /* Next file with same hash.. */
};

/* File accessor. */
struct file_acc
{
  int access;                    /* Access type. */
  struct dg_node *node;          /* Graph node of command. */
  struct file_acc *next;         /* Next accessor. */
};


void file_init (void);
//void file_add_accessor (struct redir *, struct dg_node *);
void file_add_command (union cmd *);

#endif
