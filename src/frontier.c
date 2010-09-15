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

   Written by Chen Guo, chenguo4@ucla.edu.
   Under advisement of Paul Eggert.  */

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#include "command.h"
#include "parsh.h"

#include "frontier.h"

static struct frontier frontier;
static pthread_mutex_t frnt_lock;
static pthread_cond_t frnt_cond;


/* Initialize the frontier data structure. */
void
frontier_init (void)
{
  frontier.run_list = NULL;
  frontier.run_next = NULL;
  frontier.tail = NULL;
  frontier.eof = false;
  pthread_mutex_init (&frnt_lock, NULL);
  pthread_cond_init (&frnt_cond, NULL);
}


/* Mutex control. */
void
frontier_lock (void)
{
  pthread_mutex_lock (&frnt_lock);
  DBG("MUTEX LOCKED\n");
}

void
frontier_unlock (void)
{
  DBG("MUTEX UNLOCKED\n");
  pthread_mutex_unlock (&frnt_lock);
}


/* Add a graph node to the frontier. */
void
frontier_add (struct command *command)
{
  /* Insert NODE into runnables list. */
  if (!frontier.run_list)
    {
      frontier.run_list = command;
      frontier.run_next = command;
    }
  else if (!frontier.run_next)
    {
      frontier.run_next = command;
    }
  /* Set NODE as TAIL. */
  if (frontier.tail)
    {
      command->prev = frontier.tail;
      frontier.tail->next = command;
    }
  frontier.tail = command;

  /* Signal threads waiting on cond var. */
  DBG("FRONTIER_ADD: cond signal\n");
  pthread_cond_signal (&frnt_cond);
}


/* Return RUN NEXT node for evaluation. */
struct command *
frontier_run (void)
{
  FRNT_LOCK;
  struct command *run;

  /* Conditional wait until RUN NEXT is available. */
  while (!frontier.run_next)
    {
      DBG("FRONTIER_RUN: waiting.\n");
      pthread_cond_wait (&frnt_cond, &frnt_lock);
      DBG("FRONTIER_RUN: got run_next: %p\n", frontier.run_next);
    }
  run = frontier.run_next;

  /* Update frontier. */
  frontier.run_next = run->next;

  FRNT_UNLOCK;
  return run;
}
