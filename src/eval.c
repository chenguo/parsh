/* Evaluate shell commands.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "command.h"
#include "parsh.h"

#include "eval.h"


static void forkexec (union command *, char **);

/* TODO: specify return value. I have a feeling this function needs one. */
/* Evalute a command. */
void
eval_cmd (struct dg_node *frontier_node)
{
  int argc;
  char **argv;
  struct arglist *args;
  int i;

  DBG("EVAN_CMD: command: %s\n", frontier_node->cmd->ccmd.cmd);
  /* Construct argument list. Start ARGC at 1 to account for
     the command itself. */
  args = frontier_node->cmd->ccmd.args;
  for (argc = 1; args; args = args->next, argc++)
    DBG("EVAL_CMD: arg %s\n", args->arg);
  DBG("EVAL_CMD: argc: %d\n", argc);

  /* Print each file redirection. */
  struct redir *redirp = frontier_node->cmd->ccmd.redirs;
  DBG ("EVAL_CMD: output files: ");
  while (redirp)
    {
      DBG ("%s ", redirp->file);
      redirp = redirp->next;
    }
  DBG ("\n");

  /* Copy arguments into ARGV. */
  argv = malloc (sizeof *argv * argc + 1);
  argv[0] = frontier_node->cmd->ccmd.cmd;
  args = frontier_node->cmd->ccmd.args;
  for (i = 1; i < argc; i++)
    {
      argv[i] = args->arg;
      args = args->next;
    }
  argv[i] = NULL;

  forkexec (frontier_node->cmd, argv);
}


/* Execute the command. */
static void
forkexec (union command *command, char **argv)
{
  struct ccmd *cmd = (struct ccmd *) command;

  pid_t pid = fork ();
  if (pid == 0)
    {
      char pwd[] = "/home/chen/Coding/Parsh/test/";

      /* Handle redirections. */
      struct redir *redirs;
      for (redirs = cmd->redirs; redirs; redirs = redirs->next)
        {
          /* Allocate space for file name. */
          size_t pwd_len = strlen (pwd);
          char *fname = malloc (strlen (redirs->file) + pwd_len + 1);
          strcpy (fname, pwd);
          strcpy (fname + pwd_len, redirs->file);

          /* Open a file. First get name.*/
          /* TODO: correctly set flags.
             TODO: check error codes. */
          int fd = open (fname, O_WRONLY | O_CREAT);
          DBG("File %s opened on descriptor %d.\n", fname, fd);

          /* TODO: correctly set dup fds. */
          dup2 (fd, STDOUT_FILENO);
        }

      char **env = {NULL};
      execve (cmd->cmd, argv, env);
    }
  else if (pid < 0) 
    {
      abort ();
    }
}
