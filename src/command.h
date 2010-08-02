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

   Written by Chen Guo, chenguo4@ucla.edu.  */

#ifndef COMMAND_H
#define COMMAND_H

/* Command types. TODO: Make exhaustive. */
enum
  {
    COMMAND,
    IF,
    FOR,
    WHILE,
    CASE,
    PIPE,
    SUBSHELL
  };

enum
  {
    FILE_IN,
    FILE_OUT,
    FILE_CLOBBER,
    FILE_APPEND,
    DUPE_IN,
    DUPE_OUT
  };

union command;

/* Arguments. */
struct arglist
{
  char *arg;                     /* Argument. */
  struct arglist *next;          /* Next argument. */
};

/* Redirections. */
struct redir
{
  int type;                    /* Redirection type. */
  char *file;                    /* File name. */
  int fd;                        /* File descriptor number. */
  struct redir *next;            /* Next redirection. */
};

/* Regular command. */
struct ccmd
{
  int type;                      /* Command type. */
  char *cmd;                     /* The command. */
  struct arglist *args;          /* List of arguments. */
  struct redir *redir;           /* List of redirections. */
};

/* If. */
struct cif
{
  int type;                 /* Command type. */
  union command *cif_cond;       /* If command condition. */
  union command *cif_then;       /* If command then part. */
  union command *cif_else;       /* If command else part. */
};

/* Command types. */
union command
{
  int type;
  struct ccmd ccmd;
  struct cif cif;
};

#endif
