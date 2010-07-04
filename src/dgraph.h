/* Header for dependency graph.
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

#ifndef DGRAPH_H
#define DGRAPH_H


struct dg_node;

/* List of graph nodes. */
struct dg_list
{
  struct dg_node *node;          /* Dependency graph node. */
  struct dg_list *next;          /* Next node in list. */
};

/* Dependency graph node. */
struct dg_node
{
  int type;                      /* Type of command. */
  int dependencies;              /* Dependencies. */
  struct dg_list *dependents;    /* Dependent nodes. */
  struct dg_node *parent;        /* Parent node. */
  /* Command. */
  /* Loop control. */
  unsigned long iteration;       /* Iteration of parent loop. */
  int nest;                      /* Nest level of command. */
  /* Frontier control. */
  struct dg_node *next;          /* Next node in frontier. */
  struct dg_node *prev;          /* Previous node in frontier. */
};

/* Frontier. */
struct dg_frontier
{
  struct dg_node *run_list;      /* Running/runnable commands. */
  struct dg_node *run_next;      /* Next runnable command. */
  struct dg_node *tail;          /* Tail of frontier. */
  int eof;                       /* End of file flag. */
  pthread_mutex_t *dg_lock;      /* Directed graph lock. */
  pthread_cond_t *dg_cond;       /* Conditional variable for lock. */
};

#endif
