/* Header for shell variables.
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

#ifndef VAR_H
#define VAR_H

/* TODO: should ALL of these be public? */

/* Accessor of variable. */
struct var_acc
{
  struct var_acc *next;          /* Next accessor. */
  struct command *acc;           /* Accessor. */
  //union node *arg;
};

/* Variable state. */
struct var_state
{
  struct var_state *next;        /* Next state. */
  struct var_state *prev;        /* Previous state. */
  char *val;                     /* Value. */
  int accessors;                 /* Number of accessors. */
  struct var_acc *acc_list;      /* List of accessors, only full when
                                    VAL is NUL. */
};

/* Variable hash table entry. */
struct var
{
  struct var *next;              /* Next entry with same hash. */
  char *name;                    /* Variable name. */
  struct var_state *head;        /* Oldest active state. */
  struct var_state *tail;        /* Newest active state. */
};

/* Master chronological states list. */
struct var_state_list
{
  struct var_state *state;       /* Variable state. */
  struct state_list *next;       /* Next oldest state. */
};


void var_init (void);
struct var_state * var_create_state (const char *);
void var_write_state (struct var_state *, const char *);
void var_queue_state (struct command *, struct var_state *);
struct var_state * var_read_state (const char *);

#endif
