/* Parser.
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

#include <stdio.h>
#include <stdlib.h>

#define PROMPT "$ "
#define DEFAULT_INBUF_SIZE 1024

char *inbuf;
static int bufsiz;

static void readline (FILE *input);


/* Initialize the parser. */
void init_parser (FILE *input)
{
  /* Initialize input buffer. TODO: remove arbitrary linesize limit. */
  bufsiz = DEFAULT_INBUF_SIZE;
  inbuf = malloc (bufsiz);

  /* Open input stream. */
  /* fopen (input, 'r'); */
}

/* Print prompt. */
void print_prompt (void)
{
  printf (PROMPT);
}

/* Parse a command. */
void parse_command (FILE *input)
{
  readline (input);
}

/* Read a token of the command. Return the length of the line
   read. */
static void readline (FILE *input)
{
  int status;
  if (!(status = fgets (inbuf, bufsiz, input)))
    abort ();
}
