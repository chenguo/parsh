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

#define DEFAULT_JOBTAB_SIZE 16


// TODO: job lock
struct job *jobtab;
size_t jobtab_size;

void
job_init ()
{
  jobtab = calloc (DEFAULT_JOBTAB_SIZE * sizeof *jobtab);
  size_t i;
  for (i = 0; i < DEFAULT_JOBTAB_SIZE; i++)
    {
      jobtab[i] = -1;
    }
}

/* Add a job to the table.
   TODO: table already full case? */
void
job_add (pid_t pid, struct command *command)
{
  size_t i = 0;

  /* Find free spot in job table. */
  do
    {
      if (jobtab[i].pid == -1)
        break;
    }
  while (++i < jobtab_size);

  /* Set job fields. */
  jobtab[i].pid = pid;
  jobtab[i].command = command;
}

void
job_free (pid_t pid)
{
  size_t i = 0;

  /* Find job in table. */
  do
    {
      /* Free the job. */
      if (jobtab[i].pid == pid)
        {
          jobtab[i].pid = -1;
          jobtab[i].command = NULL;
        }
    }
  while (++i < jobtab_size);
}
