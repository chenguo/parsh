/* Header for commands.
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

#ifndef COMMAND_H
#define COMMAND_H

#include "parsh.h"

/* Command tree types. TODO: Make exhaustive. */
enum
  {
    CT_COMMAND,
    CT_IF,
    CT_FOR,
    CT_WHILE,
    CT_CASE,
    CT_PIPE,
    CT_SUBSHELL,
    CT_SEMICOLON
  };

/* Redirection type + some other symbols. */
enum
  {
    NO_REDIR,
    FILE_IN,
    FILE_OUT,
    FILE_CLOBBER,
    FILE_APPEND,
    DUPE_IN,
    DUPE_OUT,
    BACKGND,
    SEMICOLON
  };

union cmdtree;
struct command;

/* Arguments. */
struct arglist
{
  char *arg;                     /* Argument. */
  struct arglist *next;          /* Next argument. */
};

/* Redirections. */
struct redir
{
  int type;                      /* Redirection type. */
  char *file;                    /* File name. */
  int fd;                        /* File descriptor number. */
  struct redir *next;            /* Next redirection. */
};

/* Regular command. */
struct ccmd
{
  int type;                      /* Command type. */
  char *cmdstr;                  /* The command. */
  struct arglist *args;          /* List of arguments. */
  struct redir *redirs;          /* List of redirections. */
};

/* If. */
struct cif
{
  int type;                      /* Command type. */
  struct command *cif_cond;      /* If command condition. */
  struct command *cif_then;      /* If command then part. */
  struct command *cif_else;      /* If command else part. */
};

/* Semicolon separated. */
struct csemi
{
  int type;                      /* Command type. */
  struct command *cmd1;          /* Command 1. */
  struct command *cmd2;          /* Command 2. */
};

/* Command types. */
union cmdtree
{
  int type;
  struct ccmd ccmd;
  struct cif cif;
  struct csemi csemi;
};

/* Command structure for interaction with file hash. */
struct command
{
  int dependencies;              /* Dependencies. */
  union cmdtree *cmdtree;        /* Command tree. */
  struct list files;             /* Files accessed. */
  struct command *parent;        /* Parent node. */
  /* Loop control. */
  unsigned long iter;            /* Iteration of parent loop. */
  int nest;                      /* Nest level of command. */
  /* Frontier control. */
  struct command *next;          /* Next node in frontier. */
  struct command *prev;          /* Previous node in frontier. */
};

struct command * ct_alloc (int, struct command *);
struct redir * ct_extract_redirs (union cmdtree *);

#endif /* COMMAND_H */
