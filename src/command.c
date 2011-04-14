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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "file.h"

//static struct redir * ct_copy_redirs (struct redir *);

/* Allocate a command tree structure. */
struct command *
ct_alloc (int ct_type, struct command *parent)
{
  struct command *cmd = calloc (1, sizeof (struct command));
  if (parent)
    {
      cmd->parent = parent;
      parent->children++;
    }
  cmd->files.size = sizeof (struct file_list);
  cmd->status = -1;

  switch (ct_type)
    {
    case CT_COMMAND:
      cmd->cmdtree = (union cmdtree *) calloc (1, sizeof (struct ccmd));
      break;
    case CT_IF:
      cmd->cmdtree = (union cmdtree *) calloc (1, sizeof (struct cif));
      cmd->cmdtree->cif.stage = IF_COND;
      break;
    case CT_FOR:
      break;
    case CT_WHILE:
      cmd->cmdtree = (union cmdtree *) calloc (1, sizeof (struct cwhile));
      cmd->cmdtree->cwhile.cond_eval = true;
      cmd->cmdtree->cwhile.iteration = 0;
      break;
    case CT_UNTIL:
      cmd->cmdtree = (union cmdtree *) calloc (1, sizeof (struct cwhile));
      cmd->cmdtree->cwhile.cond_eval = false;
      cmd->cmdtree->cwhile.iteration = 0;
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

/* Copy a set of redirections.
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
*/

 /* Decrease the child count.
    Not entirely sure if this should be static. Leave exposed for now, if
    no uses come up, then make this static. */
void
ct_dec_child_count (struct command *command)
{
  if (--command->children == 0)
    ct_free (command);
}

/* Process a command's parent. */
static void
ct_parent_process (struct command *command)
{
  struct command *parent = command->parent;
  DBG("CT PARENT PROCESS: PARENT %p\n", parent);
  if (!parent)
    return;

  union cmdtree *cmdtree = parent->cmdtree;
  switch (cmdtree->type)
    {
    case CT_IF:
      /* TODO: if one part of the condition evaluates to true, we immediately
         know to insert the THEN part into the frontier. Right now, we're
         waiting to evaluate the entirety of the conditional part. A fairly
         obvious optimization would be to immediately insert the THEN part
         upon encountering one portion of the conditional that evaluates as
         true, rather than wait for the whole conditional to be evaluated. */
      if (cmdtree->cif.stage == IF_COND)
        {
          if (command->status == 0)
            {
              file_command_process (cmdtree->cif.cif_then, FILE_INSERT);
              cmdtree->cif.stage = IF_THEN;
            }
          else
            {
              file_command_process (cmdtree->cif.cif_else, FILE_INSERT);
              cmdtree->cif.stage = IF_ELSE;
            }
        }
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
      if (parent->status != 0)
        parent->status = command->status;
      break;
    }
  ct_dec_child_count (parent);
}


/* Free a command structure. */
void
ct_free (struct command *command)
{
  DBG("CT FREE\n");
  ct_parent_process (command);
  /* TODO: thoroughly free the command and command tree. */
}
