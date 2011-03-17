/* File list and dependency.
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

static void file_command_add (union cmdtree *);
static void file_command_list_add (struct file *, struct list *);
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

/* Create hash entry for a FILE. */
static struct file **
file_file_add (char *file)
{
  // TODO: Absolute path.
  /* Allocate new file struct. */
  struct file *new_file = malloc (sizeof *new_file);
  new_file->access = NO_ACCESS;
  new_file->path = file;
  new_file->accessors = NULL;
  new_file->acc_tail = NULL;

  /* Place struct in hash table. */
  struct file **fpp = file_hash (file);
  new_file->next = *fpp;
  *fpp = new_file;
  return fpp;
}

/* Check if a new accessor to a FILE is blocked. */
static bool
file_file_dep_check (struct file* file, struct file_acc *new_acc)
{
  DBG("FILE DEP CHECK: %s\n", file->path);
  DBG("  file: %p\n", file);
  DBG("  new_acc: %p\n", new_acc);
  DBG("  access type: %d\n", new_acc->access);

  /* Base case: accessor has access. */
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
      DBG("accessors: %p\n", acc);
      while (acc && acc != new_acc)
        {
          /* Any write will block. */
          if (acc->access == WRITE_ACCESS)
            return true;
          acc = acc->next;
        }
    }

  return false;
}

/* Add an accessor ACC to a FILE in the hash. If the FILE is not yet in hash,
   add it to the file hash.  */
static bool
file_file_add_accessor (struct redir *redir, struct command *acc,
                   struct file *file)
{
  DBG("FILE ADD ACCESSOR: %s\n", file->path);

  /* Create new accessor struct. */
  struct file_acc *new_acc = malloc (sizeof *new_acc);
  switch (redir->type)
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

  /* Add new accessor to accessor list. */
  if (file->acc_tail)
    file->acc_tail->next = new_acc;
  else
    file->accessors = new_acc;

  file->acc_tail = new_acc;

  /* Check dependencies. */
  return file_file_dep_check (file, new_acc);
}

/* Remove an accessor from a file in the hash. If removing this accessor
   allows other accessors to run, put them in the frontier. */
static void
file_file_remove_accessor (struct file *file, struct command *acc)
{
  /* Look through the file's accessors. */
  struct file_acc *accessor = file->accessors;
  struct file_acc **acc_pp = &file->accessors;

  while (accessor)
    {
      /* Find the accessor representing ACC. */
      if (accessor->cmd == acc)
        {
          DBG("  accesssor to be removed found\n");
          /* If next accessor is blocked, and removing this accessor
             would unblock it, then put it into the fronter. This means:
             1) Accessor is writer.
             2) Accessor is the only reader, next accessor is writer.
          */

          DBG("  Conditions:\n");
          DBG("    accessor->next: %p\n", accessor->next);
          DBG("    access: %d\n", accessor->access);
          DBG("    accessor: %p\n", accessor);
          DBG("    file->accessors: %p\n", file->accessors);
          if (accessor->next &&
              (accessor->access == WRITE_ACCESS || accessor->access == RW_ACCESS
               || (accessor->access == READ_ACCESS
                   && accessor->next->access != READ_ACCESS
                   && accessor == file->accessors)))
            {
              DBG("    deps: %d\n", accessor->next->cmd->dependencies);
              if (--accessor->next->cmd->dependencies == 0)
                frontier_add (accessor->next->cmd);

            }
          /* Update previous accessor's NEXT field. */
          *acc_pp = accessor->next;

          free (accessor);
          break;
        }
      acc_pp = &accessor->next;
      accessor = accessor->next;
    }
}

/* Process a CMDTREE for addition into the hash table. */
void
file_command_process (struct command *cmd)
{
  /* TODO: handle rest of unsupported command structures. */
  switch (cmd->cmdtree->type)
    {
    case CT_IF:
      /* TODO: handle IF. */
      return;

    case CT_SEMICOLON:
      file_command_process (cmd->cmdtree->csemi.cmd1);
      file_command_process (cmd->cmdtree->csemi.cmd2);
      return;

    case CT_COMMAND:
      /* Regular command. Process below. */
      file_command_add (cmd->cmdtree);
      return;

    default: return;
    }
}

/* Add a command's file accesses to the hash table. */
static void
file_command_add (union cmdtree *new_cmdtree)
{
  /* Allocate command structure to hold command tree. */
  struct command *new_command = calloc (1, sizeof *new_command);
  new_command->cmdtree = new_cmdtree;
  new_command->files.size = sizeof (struct file_list);

  FILE_LOCK;
  /* Check dependencies. */
  struct redir *redirs = ct_extract_redirs (new_cmdtree);

  /* For each file. */
  while (redirs)
    {
      DBG("FILE ADD COMMAND: file: %s\n", redirs->file);

      /* Find the file being accessed. If not yet hashed, hash it. */
      struct file *file = *file_find (file_hash (redirs->file), redirs->file);
      if (!file)
        file = *file_file_add (redirs->file);

      /* Add command as an accessor. */
      if (file_file_add_accessor (redirs, new_command, file))
        new_command->dependencies++;

      /* Also add file to command's file-accessed list. */
      file_command_list_add (file, &new_command->files);

      redirs = redirs->next;
    }

  DBG("FILE ADD COMMAND: dependencies: %d\n", new_command->dependencies);

  /* Add to frontier if no dependencies. */
  if (new_command->dependencies == 0)
    frontier_add (new_command);
  FILE_UNLOCK;
}

/* Insert a command into file hash table. For each file the command accesses,
   the command will be inserted after the currently runnable commands, but
   before every other command.
   Note that if the currently runnable commands have read access, and the
   inserted command also has read access, the inserted command will become
   runnable. */
void
file_command_insert (struct command *command)
{
  FILE_LOCK;
  struct redir *redirs = ct_extract_redirs (command->cmdtree);

  /* For each file. */
  while (redirs)
    {
      DBG("FILE INSERT COMMAND: file: %s\n", redirs->file);

      /* Find the file to be accessed. */
      struct file *file = *file_find (file_hash (redirs->file), redirs->file);
      if (!file)
        DBG("FILE INSERT COMMAND: error: file not hashed\n");

      /* Add command to frontier if both runnables and command are reads. */
      if (!file->accessors ||
          (redirs->type == FILE_IN && file->accessors->access == READ_ACCESS))
        frontier_add (command);

      /* Skip through runnable commands.
         1. Read access: iterate until end.
         2. Write access: insert after first accessor.
         3. No accessor: just set as accessor. */
      struct file_acc *acc = file->accessors;
      struct file_acc **accp = &file->accessors;
      while (acc)
        {
          /* Iterate through accessors until write access is found. */
          accp = &acc->next;
          if (acc->access != READ_ACCESS)
            {
              /* If writer is first accessor, skip over it. Otherwise,
                 insert before it. */
              if (acc == file->accessors)
                acc = acc->next;
              break;
            }
          acc = acc->next;
        }

      /* Create accessor for command and insert into accessor list. */
      struct file_acc *new_acc = malloc (sizeof *new_acc);
      if (redirs->type == FILE_IN)
        new_acc->access = READ_ACCESS;
      else if (redirs->type == FILE_OUT && redirs->type == FILE_CLOBBER
               && redirs->type == FILE_APPEND)
        new_acc->access = WRITE_ACCESS;
      new_acc->cmd = command;
      new_acc->next = acc;
      *accp = new_acc;

      redirs = redirs->next;
    }

  FILE_UNLOCK;
}

/* Remove a command from the file hash table. */
void
file_command_remove (struct command* command)
{
  FILE_LOCK;
  struct redir *redirs = ct_extract_redirs (command->cmdtree);

  /* For each file. */
  while (redirs)
    {
      DBG("FILE REMOVE COMMAND: file: %s\n", redirs->file);

      /* Find the file being accessed. */
      struct file *file = *file_find (file_hash (redirs->file), redirs->file);
      if (!file)
        DBG("FILE REMOVE COMMAND: error: file not hashed\n");

      /* Remove command as an accessor to the file. */
      file_file_remove_accessor (file, command);

      redirs = redirs->next;
    }

  /* TODO: thoroughly free command. */
  FILE_UNLOCK;
}

/* Add a file struct to a command's file access list. */
static void
file_command_list_add (struct file *file, struct list *file_list)
{
  struct file_list *flist =
    (struct file_list *) list_append (file_list);
  flist->file = file;
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
