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

#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "file.h"
#include "parsh.h"

#include "jobs.h"

#define DEFAULT_JOBTAB_SIZE 16

// TODO: job lock
static struct job *jobtab;
static size_t jobtab_size;
static pthread_mutex_t job_lock;

#define JOB_LOCK   {pthread_mutex_lock (&job_lock);}
#define JOB_UNLOCK {pthread_mutex_unlock (&job_lock);}

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
  pthread_mutex_init (&job_lock, NULL);
}

/* Add a job to the table.
   TODO: table already full case? */
void
job_add (pid_t pid, struct command *command)
{
  JOB_LOCK;

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
  DBG("JOB_ADD: Process %d set as job %ld\n", pid, i);

  JOB_UNLOCK;
}

static void
job_free (pid_t pid, int status)
{
  DBG("JOB_FREE: freeing process %d\n", pid);

  JOB_LOCK;
  /* Find job in table. */
  size_t i = 0;
  do
    {
      /* Free the job. */
      if (jobtab[i].pid == pid)
        {
          /* Remove the command from the file hash. */
          file_command_remove (jobtab[i].command, status);
          jobtab[i].pid = -1;
          jobtab[i].command = NULL;
        }
    }
  while (++i < jobtab_size);
  JOB_UNLOCK;
}

/* Wait on child processes, return PID of finished ones. */
pid_t
job_wait ()
{
  pid_t pid = -1;
  int status;
  while (pid <= 0)
    {
      pid = waitpid (-1, &status, 0);
    }
  DBG("JOB_WAIT: pid: %d, status %d\n", pid, status);
  job_free (pid, status);
  return pid;
}
