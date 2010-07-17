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
    REDIRECT,
    IF,
    FOR,
    WHILE,
    CASE,
    PIPE,
    SUBSHELL
  };

union command;
struct arglist;

/* Regular command. */
struct ccmd
{
  int type;                      /* Type. */
  char *cmd;                     /* The command. */
  struct arglist *args;          /* List of arguments. */
};

/* If. */
struct cif
{
  int type;
};

struct arglist
{
  char *arg;                     /* Argument. */
  struct arglist *next;          /* Next argument. */
};

/* Command types. */
union command
{
  int type;
  struct ccmd ccmd;
  struct cif cif;
};

#endif
