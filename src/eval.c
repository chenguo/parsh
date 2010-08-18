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

   Written by Chen Guo, chenguo4@ucla.edu.
   Under advisement of Paul Eggert.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
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
      /* Handle redirections. */
      struct redir *redirs;
      for (redirs = cmd->redirs; redirs; redirs = redirs->next)
        {
          int fd_dup;
          int fd;
          int flags;
          /* TODO: correctly set mode. */
          mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

          /* Open files. */
          /* TODO: Check error codes. */
          /* TODO: file descriptors, such as 2> */
          DBG("FORKEXEC: type %d\n", redirs->type);
          switch (redirs->type)
            {
            case FILE_IN:
              flags = O_RDONLY;
              fd_dup = STDIN_FILENO;
              break;

            case FILE_OUT:
              /* TODO: Respect noclobber. */
              flags = O_WRONLY | O_CREAT | O_TRUNC;
              fd_dup = STDOUT_FILENO;
              break;

            case FILE_CLOBBER:
              flags = O_WRONLY | O_CREAT | O_TRUNC;
              fd_dup = STDOUT_FILENO;
              break;

            case FILE_APPEND:
              flags = O_WRONLY | O_CREAT | O_APPEND;
              fd_dup = STDOUT_FILENO;
              break;
            }

          fd = open (redirs->file, flags, mode);
          DBG("File %s opened on descriptor %d.\n", redirs->file, fd);
          dup2 (fd, fd_dup);
        }

      char **env = {NULL};
      execve (cmd->cmd, argv, env);
    }
  else if (pid < 0) 
    {
      abort ();
    }
}
