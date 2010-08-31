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

#include "file.h"
#include "parsh.h"

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

/* Add an accessor to a file/files in the hash. */
void
file_add_accessor (const struct dg_node *graph_node)
{
  /* Parse file accesses. */

  /* Add graph_node as accessor. If no file in hash,
     create an entry. */
}

/* Create hash entry for a file. */
static void
file_add (char *file)
{
  // TODO: Absolute path.
  struct file *new_file = malloc (sizeof *new_file);
  new_file->path = file;
  new_file->accessors = NULL;
  struct file **fpp = file_hash (file);
  new_file->next = *fpp;
  *fpp = new_file;
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
