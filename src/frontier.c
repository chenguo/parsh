/* Dependency graph frontier.
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

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#include "dgraph.h"
#include "parsh.h"

#include "frontier.h"

static struct frontier frontier;
static pthread_mutex_t dg_lock;
static pthread_cond_t dg_cond;


/* Initialize the frontier data structure. */
void
frontier_init (void)
{
  frontier.run_list = NULL;
  frontier.run_next = NULL;
  frontier.tail = NULL;
  frontier.eof = false;
  pthread_mutex_init (&dg_lock, NULL);
  pthread_cond_init (&dg_cond, NULL);
}


/* Mutex control. */
void
frontier_lock (void)
{
  pthread_mutex_lock (&dg_lock);
  DBG("MUTEX LOCKED\n");
}

void
frontier_unlock (void)
{
  DBG("MUTEX UNLOCKED\n");
  pthread_mutex_unlock (&dg_lock);
}


/* Add a graph node to the frontier. */
void
frontier_add (struct dg_node *node)
{
  /* If RUN LIST is empty, NODE is the only command
     in the graph. */
  if (!frontier.run_list)
    {
      frontier.run_list = node;
      frontier.run_next = node;
    }
  /* Set NODE as TAIL. */
  if (frontier.tail)
    {
      node->prev = frontier.tail;
      frontier.tail->next = node;
    }
  frontier.tail = node;

  /* Signal threads waiting on cond var. */
  DBG("FRONTIER_ADD: cond signal\n");
  pthread_cond_signal (&dg_cond);
}


/* Return RUN NEXT node for evaluation. */
struct dg_node *
frontier_run (void)
{
  DG_LOCK;
  struct dg_node *run;

  /* Conditional wait until RUN NEXT is available. */
  while (!frontier.run_next)
    {
      DBG("FRONTIER_RUN: waiting.\n");
      pthread_cond_wait (&dg_cond, &dg_lock);
      DBG("FRONTIER_RUN: got run_next\n");
    }
  run = frontier.run_next;

  /* Update frontier. */
  frontier.run_next = run->next;

  DG_UNLOCK;
  return run;
}
