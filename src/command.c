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

/* Extracts list of redirections from command tree. */
struct redir *
ct_extract_redirs (union cmdtree *cmdtree)
{
  switch (cmdtree->type)
    {
    case COMMAND:
      /* For regular command, should be safe to take as is. */
      return cmdtree->ccmd.redirs;
      //return ct_copy_redirs (cmdtree->ccmd.redirs);

    }
  return NULL;
}

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
