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

   Written by Chen Guo, chenguo4@ucla.edu.  */

#ifndef FRONTIER_H
#define FRONTIER_H

#include <stdbool.h>
#include <phread.h>

#include "dgraph.h"

/* Frontier. */
struct dg_frontier
{
  struct dg_node *run_list;      /* Running/runnable commands. */
  struct dg_node *run_next;      /* Next runnable command. */
  struct dg_node *tail;          /* Tail of frontier. */
  bool eof;                       /* End of file flag. */
  pthread_mutex_t *dg_lock;      /* Directed graph lock. */
  pthread_cond_t *dg_cond;       /* Conditional variable for lock. */
};

void frontier_add (struct dg_node *);

#endif
