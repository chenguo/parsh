/* Header for dependency graph frontier.
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

#ifndef FRONTIER_H
#define FRONTIER_H

#include <stdbool.h>
#include <pthread.h>

#include "dgraph.h"

/* Frontier. */
struct frontier
{
  struct dg_node *run_list;      /* Running/runnable commands. */
  struct dg_node *run_next;      /* Next runnable command. */
  struct dg_node *tail;          /* Tail of frontier. */
  bool eof;                      /* End of file flag. */
};

void frontier_init (void);
void frontier_lock (void);
void frontier_unlock (void);
void frontier_add (struct dg_node *);
struct dg_node * frontier_run (void);

#define DG_LOCK frontier_lock ();
#define DG_UNLOCK frontier_unlock ();

#endif
