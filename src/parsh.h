/* Common definitions and functions.
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

#ifndef PARSH_H
#define PARSH_H

#include <stdio.h>

#define DEBUG

FILE *dbg;

#ifdef DEBUG
#define DBG(msg...) fprintf (stderr, msg);
#else
#define DBG(msg...)
#endif

struct list
{
  void *head;                    /* Head node of list. */
  void *tail;                    /* Last node of list. */
  size_t size;                   /* Size of each node. */
};

struct node
{
  void *next;                    /* Psuedo struct for list node. */
};

void *list_append (struct list *);
void *list_behead (struct list *);
char *strncpy_nul (const char *, size_t);

#endif /* PARSH_H */
