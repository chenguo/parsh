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
#include <unistd.h>

#include "command.h"
#include "parsh.h"

#include "eval.h"


static void forkexec (char *cmd, char **argv);

/* TODO: specify return value. I have a feeling this function needs one. */
/* Evalute a command. */
void
eval_cmd (struct dg_node *command)
{
  int argc;
  char **argv;
  struct arglist *args;
  int i;

  /* Construct argument list. Start ARGC at 1 to account for
     the command itself. */
  args = command->cmd->ccmd.args;
  for (argc = 1; args; args = args->next, argc++) DBG("EVAL_CMD: arg %s\n", args->arg);
  DBG("EVAL_CMD: argc: %d\n", argc);


  /* Copy arguments into ARGV. */
  argv = malloc (sizeof *argv * argc + 1);
  argv[0] = command->cmd->ccmd.cmd;
  args = command->cmd->ccmd.args;
  for (i = 1; i < argc; i++)
    {
      argv[i] = args->arg;
      args = args->next;
    }
  argv[i] = NULL;

  forkexec (command->cmd->ccmd.cmd, argv);
}


/* Execute the command. */
static void
forkexec (char *cmd, char **argv)
{
  pid_t pid = fork ();
  if (pid == 0)
    {
      char **env = {NULL};
      execve (cmd, argv, env);
    }
  else if (pid < 0) 
    {
      abort ();
    }
}
