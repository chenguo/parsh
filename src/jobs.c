/* Job control.
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
#include <sys/wait.h>

#include "file.h"
#include "parsh.h"

#include "jobs.h"

#define DEFAULT_JOBTAB_SIZE 16


// TODO: job lock
struct job *jobtab;
size_t jobtab_size;

void
job_init ()
{
  jobtab = calloc (DEFAULT_JOBTAB_SIZE, sizeof *jobtab);
  jobtab_size = DEFAULT_JOBTAB_SIZE;
  size_t i;
  for (i = 0; i < jobtab_size; i++)
    {
      jobtab[i].pid = -1;
    }
}

/* Add a job to the table.
   TODO: table already full case? */
void
job_add (pid_t pid, struct command *command)
{
  /* Find free spot in job table. */
  size_t i = 0;
  do
    {
      if (jobtab[i].pid == -1)
        break;
    }
  while (++i < jobtab_size);

  /* Set job fields. */
  jobtab[i].pid = pid;
  jobtab[i].command = command;
  DBG("JOB_ADD: Process %d set as job %d\n", pid, i);
}

void
job_free (pid_t pid)
{
  DBG("JOB_FREE: freeing process %d\n", pid);

  /* Find job in table. */
  size_t i = 0;
  do
    {
      /* Free the job. */
      if (jobtab[i].pid == pid)
        {
          /* Remove the command from the file hash. */
          file_command_remove (jobtab[i].command);
          jobtab[i].pid = -1;
          jobtab[i].command = NULL;
        }
    }
  while (++i < jobtab_size);
}

/* Wait on child processes, return PID of finished ones. */
pid_t
job_wait ()
{
  pid_t pid = -1;
  int printed = -1000;
  while (pid <= 0)
    {
      pid = waitpid (-1, NULL, 0);
      if ((pid == 0 || pid == -1) && printed != pid)
        {
          DBG("JOB_WAIT: pid is %d\n", pid);
          printed = pid;
        }
    }
  DBG("JOB_WAIT: pid: %d\n", pid);
  return pid;
}
