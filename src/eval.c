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
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include "command.h"
#include "jobs.h"
#include "parsh.h"
#include "var.h"

#include "eval.h"


static void forkexec (struct command *, char **);
static char *findpath (const char *);

/* Evalute a command.
   TODO: specify some return value. I have a feeling this function
   will end up needing one. */
void
eval_cmd (struct command *frontier_node)
{
  DBG("EVAL CMD\n");
  int argc;
  char **argv;
  struct arglist *args;
  int i;

  DBG("  command: %s\n", frontier_node->cmdtree->ccmd.cmdstr);
  /* Construct argument list. Start ARGC at 1 to account for
     the command itself. */
  args = frontier_node->cmdtree->ccmd.args;
  for (argc = 1; args; args = args->next, argc++)
    DBG("  arg (%s, %p)\n", args->arg, args);
  DBG("  argc: %d\n", argc);

  /* Print each file redirection. */
  struct redir *redirp = frontier_node->cmdtree->ccmd.redirs;
  DBG("  output files: ");
  while (redirp)
    {
      DBG ("%s ", redirp->file);
      redirp = redirp->next;
    }
  DBG("\n");

  /* Copy arguments into ARGV. */
  argv = malloc (sizeof *argv * argc + 1);
  argv[0] = frontier_node->cmdtree->ccmd.cmdstr;
  args = frontier_node->cmdtree->ccmd.args;
  for (i = 1; i < argc; i++)
    {
      argv[i] = args->arg;
      args = args->next;
    }
  argv[i] = NULL;

  forkexec (frontier_node, argv);
}


/* Execute the command. */
static void
forkexec (struct command *frontier_node, char **argv)
{
  struct ccmd *cmd = (struct ccmd *) frontier_node->cmdtree;

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
      char *cmdpath = findpath (cmd->cmdstr);
      DBG("CMD: %s\n", cmdpath);
      if (cmdpath)
        execve (cmdpath, argv, env);
      else
        printf ("%s: command not found.\n", cmd->cmdstr);
    }
  else if (pid > 0)
    {
      /* Create a new entry in the job table. */
      job_add (pid, frontier_node);
    }
  else
    abort ();
}

/* Resolve the path of the executable.
   TODO: Don't just read state, as $PATH will change while
   reading state only gets us the most recent $PATH. */
static char cmdbuf[PATH_MAX];
static char *
findpath (const char *exe)
{
  /* Read path. */
  struct var_state *pathstate = var_read_state ("PATH");
  char *path_start = pathstate->val;
  size_t exelen = strlen (exe);

  /* For each path, STAT the executable. */
  while (1)
    {
      /* Get a executable path. */
      size_t pathlen;
      char *path_end = strchr (path_start, ':');
      if (path_end)
          pathlen = path_end - path_start;
      else
        pathlen = strlen (path_start);
      strncpy (cmdbuf, path_start, pathlen);
      if (cmdbuf[pathlen - 1] != '/')
        cmdbuf[pathlen++] = '/';
      strncpy (cmdbuf + pathlen, exe, exelen);
      cmdbuf[pathlen + exelen] = '\0';

      /* Stat the file at the path. */
      struct stat statbuf;
      if (stat (cmdbuf, &statbuf) == 0)
        return cmdbuf;

      /* Move onto next prefix until prefixes are exhausted. */
      if (!path_end)
        break;
      else
        path_start = path_end + 1;
    }
  return NULL;
}
