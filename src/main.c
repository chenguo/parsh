/* Parsh, the Parallel Shell.
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


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "dgraph.h"

#define PROMPT "$ "

static void *eval_loop (void *);
static void *parse_loop (void *);
static void *reap_loop (void *);
static void print_prompt (void);

int main (void)
{
  pthread_t eval;
  pthread_t reap;

  pthread_create (&eval, NULL, eval_loop, NULL);  
  pthread_create (&reap, NULL, reap_loop, NULL);

  parse_loop (NULL);
}

/* Eval loop. */
static void *
eval_loop (void *ignore)
{

}

/* Parse lopp. */
static void *
parse_loop (void *ignore)
{
  FILE *input = stdin;
  union command *cmd;

  parse_init (input);


  for (;;)
    {
      print_prompt ();
      parse_input (input);
    }
}

/* Reap loop. */
static void *
reap_loop (void *ignore)
{}

/* Print prompt. */
static void
print_prompt (void)
{
  printf (PROMPT);
}
