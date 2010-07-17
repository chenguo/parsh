/* Dependency graph.
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

#include <stdlib.h>

#include "command.h"
#include "frontier.h"

#include "dgraph.h"


static struct dg_node * dg_create (union command *);

void
dg_add (union command *cmd)
{
  /* Create node for command. */
  struct dg_node *new_node = dg_create (cmd);

  /* For now, just straight add it. */
  /* Add to frontier. */
  frontier_add (new_node);
}

static struct dg_node *
dg_create (union command *cmd)
{
  /* Calloc to start with all NULL fields. */
  struct dg_node *new_node = calloc (1, sizeof *new_node);
  new_node->cmd = cmd;

  return new_node;
}
