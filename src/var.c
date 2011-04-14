/* Shell variables.
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
#include <string.h>

#include "parsh.h"

#include "var.h"

static pthread_mutex_t var_lock;
static pthread_mutexattr_t attr;

#define VAR_LOCK {pthread_mutex_lock (&var_lock);}
#define VAR_UNLOCK {pthread_mutex_unlock (&var_lock);}

/* Hash table based off of Dash's implementation. */
#define VTABSIZE 39
struct var *vartab[VTABSIZE];

const char default_path[] =
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin";

static struct var ** var_hash (const char *);
static struct var ** var_find (struct var **, const char *);


void
var_init (void)
{
  /* Create recursive mutex. */
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init (&var_lock, &attr);
  VAR_LOCK;

  /* TODO: Completely initialize the hash table, not with just $PATH. */
  struct var_state *new_state = var_create_state (default_path);
  var_write_state (new_state, default_path);
  DBG("PATH: %s\n", new_state->val);

  VAR_UNLOCK;
}

/* Create a new state. */
struct var_state *
var_create_state (const char *assnstr)
{
  /* Copy name. */
  char *eq = strchr (assnstr, '=');
  char *name = strncpy_nul (assnstr, eq - assnstr);

  /* Create a mew state. */
  struct var_state *new_state = malloc (sizeof *new_state);

  new_state->next = NULL;
  new_state->prev = NULL;
  new_state->val = NULL;
  new_state->accessors = 0;
  new_state->acc_list = 0;

  VAR_LOCK;

  /* Find the variable that's being written to. If variable doesn't
     exist, allocate it. */
  struct var **vpp = var_hash (name);
  struct var *vp = *var_find (vpp, name);
  if (!vp)
    {
      /* Create new bucket entry; put at head. */
      vp = (struct var *) malloc (sizeof (struct var));
      vp->next = *vpp;
      vp->name = name;
      vp->head = NULL;
      vp->tail = NULL;
      *vpp = vp;
    }
  else
    {
      free (name);
    }

  /* Append new state. */
  if (!vp->head)
    vp->head = new_state;
  else
    {
      /* If old state has no accessors, it is obsolete. Remove it.
         TODO: remove it. It's only safe to do this once the sequential
         assignment queue has been constructed. It's safe to clean up
         $y -> $y, but not $y -> $x -> $y, since the $x can refer to the
         older state of $y. */
      vp->tail->next = new_state;
      new_state->prev = vp->tail;
    }
  vp->tail = new_state;
  VAR_UNLOCK;
  return new_state;
}

/* Write a state. */
void
var_write_state (struct var_state *state, const char *assnstr)
{
  /* Make a copy of value. */
  char *val = strchr (assnstr, '=') + 1;
  state->val = strncpy_nul (val, strlen (val));

  VAR_LOCK;
  /* Unblock accessors. */
  struct var_acc *iter = state->acc_list;
  struct var_acc *save;
  while (iter)
    {
      /* TODO: decrease dependency count by 1. There will
         be a dgraph function for this. */

      save = iter->next;
      free (iter);
      iter = save;
    }
  VAR_UNLOCK;
}

/* Attach an accessor to a state. */
void
var_queue_state (struct command *node, struct var_state *state)
{
  VAR_LOCK;
  if (!state->val)
    {
      struct var_acc *acc = (struct var_acc *) malloc (sizeof *acc);
      acc->next = state->acc_list;
      acc->acc = node;
      state->acc_list = acc;
    }
  state->accessors++;
  VAR_UNLOCK;
}

/* Read variable state. Return NULL is variable does not exist. */
struct var_state *
var_read_state (const char *name)
{
  DBG("READ STATE\n");
  VAR_LOCK;
  struct var **vtab_entry = var_hash (name);
  struct var *var = *var_find (vtab_entry, name);
  struct var_state *state = var->tail;

  VAR_UNLOCK;
  return state;
}

/* Hash function based off of Dash's. */
static struct var **
var_hash (const char *p)
{
  unsigned int hashval;

  hashval = ((unsigned char) *p) << 4;
  while (*p)
    hashval += (unsigned char) *p++;
  return &vartab[hashval % VTABSIZE];
}

/* Find a variable in a hash bucket. */
static struct var **
var_find (struct var **vpp, const char *name)
{
  DBG(("FIND VAR\n"));
  for (; *vpp; vpp = &(*vpp)->next)
    if (strcmp ((*vpp)->name, name) == 0)
      break;
  return vpp;
}

