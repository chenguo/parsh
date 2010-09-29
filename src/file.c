/* Header for file list and dependency. 
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
#include <string.h>

#include "command.h"
#include "frontier.h"
#include "parsh.h"

#include "file.h"


static pthread_mutex_t file_lock;
static pthread_mutexattr_t attr;

#define FILE_LOCK {pthread_mutex_lock (&file_lock);}
#define FILE_UNLOCK {pthread_mutex_unlock (&file_lock);}

/* Hash table of file names. */
#define FTABSIZE 39
struct file *filetab[FTABSIZE];

static struct file ** file_hash (const char *);
static struct file ** file_find (struct file **, const char *);

void
file_init (void)
{
  /* Create recursive mutex. */
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init (&file_lock, &attr);
}

/* Create hash entry for a file. */
static struct file **
file_add (char *file)
{
  // TODO: Absolute path.
  /* Allocate new file struct. */
  struct file *new_file = malloc (sizeof *new_file);
  new_file->access = NO_ACCESS;
  new_file->path = file;
  /* Place struct in hash table. */
  struct file **fpp = file_hash (file);
  new_file->next = *fpp;
  *fpp = new_file;
  return fpp;
}

/* Check if a new accessor to a file is blocked. */
static bool
file_dep_check (struct file* file, struct file_acc *new_acc)
{
  /* Base case. */
  if (file->accessors == new_acc)
    return false;

  /* Accessor is not first in line. */
  if (new_acc->access == WRITE_ACCESS)
    {
      /* Any write is blocked. */
      return true;
    }
  else
    {
      struct file_acc *acc = file->accessors;
      while (acc != new_acc)
        {
          /* Any write will block. */
          if (acc->access == WRITE_ACCESS)
            return true;
          acc = acc->next;
        }
    }
  return false;
}

/* Add an accessor to a file in the hash. If the file is not yet in hash,
   add it to the file hash.  */
static bool
file_add_accessor (struct redir *redirs, struct command *acc)
{
  /* Create new accessor struct. */
  struct file_acc *new_acc = malloc (sizeof *new_acc);
  switch (redirs->type)
    {
    case FILE_IN:
      new_acc->access = READ_ACCESS;
      break;
    case FILE_OUT:
    case FILE_CLOBBER:
    case FILE_APPEND:
      new_acc->access = WRITE_ACCESS;
    }
  new_acc->cmd = acc;
  new_acc->next = NULL;

  /* Find the file being accessed. */
  struct file *file = *file_find (file_hash (redirs->file), redirs->file);
  if (file)
    {
      /* File already hashed. Set pointers. */
      if (file->acc_tail)
        file->acc_tail->next = new_acc;
      else
        file->accessors = new_acc;

      file->acc_tail = new_acc;
    }
  else
    {
      /* File not yet hashed. Hash it. */
      file = *file_add (redirs->file);
      file->accessors = new_acc;
      file->acc_tail = new_acc;
    }

  return file_dep_check (file, new_acc);
}

/* Add a command's file accesses to the hash table. */
void
file_add_command (union cmdtree *new_cmd)
{
  struct command *new_command = calloc (1, sizeof *new_command);
  new_command->cmdtree = new_cmd;

  FILE_LOCK;
  /* Check dependencies. */
  struct redir *redirs = ct_extract_redirs (new_cmd);

  /* For each file, add to hash. */
  while (redirs)
    {
      DBG("FILE ADD COMMAND: file: %s\n", redirs->file);
      if (file_add_accessor (redirs, new_command))
        new_command->dependencies++;
      redirs = redirs->next;
    }

  DBG("FILE ADD COMMAND: dependencies: %d\n", new_command->dependencies);

  /* Add to frontier if no dependencies. */
  if (new_command->dependencies == 0)
    frontier_add (new_command);
  FILE_UNLOCK;
}

/* Hash function based off of Dash's. */
static struct file **
file_hash (const char *p)
{
  unsigned int hashval;

  hashval = ((unsigned char) *p) << 4;
  while (*p)
    hashval += (unsigned char) *p++;
  return &filetab[hashval % FTABSIZE];
}

/* Find a variable in a hash bucket. */
static struct file **
file_find (struct file **fpp, const char *name)
{
  for (; *fpp; fpp = &(*fpp)->next)
    if (strcmp ((*fpp)->path, name) == 0)
      break;
  return fpp;
}
