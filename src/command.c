/* Commands.
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

#include <stdlib.h>
#include <string.h>

#include "command.h"

static struct redir * ct_copy_redirs (struct redir *);

/* Allocate a command tree structure. */
struct command *
ct_alloc (int ct_type, struct command *parent)
{
  struct command *cmd = calloc (1, sizeof (struct command));
  cmd->parent = parent;

  switch (ct_type)
    {
    case CT_COMMAND:
      cmd->cmdtree = (union cmdtree *) calloc (1, sizeof (struct ccmd));
      break;
    case CT_IF:
      cmd->cmdtree = (union cmdtree *) calloc (1, sizeof (struct cif));
      break;
    case CT_FOR:
      break;
    case CT_WHILE:
      break;
    case CT_CASE:
      break;
    case CT_PIPE:
      break;
    case CT_SUBSHELL:
      break;
    case CT_SEMICOLON:
      cmd->cmdtree = (union cmdtree *) calloc (1, sizeof (struct csemi));
      break;
    default:
      break;
    }
  if (cmd->cmdtree)
    cmd->cmdtree->type = ct_type;
  return cmd;
}

/* Extracts list of redirections from command tree. */
struct redir *
ct_extract_redirs (union cmdtree *cmdtree)
{
  switch (cmdtree->type)
    {
    case CT_COMMAND:
      /* For regular command, should be safe to take as is. */
      return cmdtree->ccmd.redirs;
      //return ct_copy_redirs (cmdtree->ccmd.redirs);

    }
  return NULL;
}

/* Copy a set of redirections. */
static struct redir *
ct_copy_redirs (struct redir *redir)
{
  DBG("CT COPY REDIRS: redir %p\n", redir);
  struct redir *new_redir = NULL;
  struct redir **new_redirp = &new_redir;
  while (redir)
    {
      *new_redirp = malloc (sizeof (*new_redir));
      memcpy (*new_redirp, redir, sizeof (*redir));
      redir = redir->next;
      new_redirp = &(*new_redirp)->next;
    }
  return new_redir;
}
