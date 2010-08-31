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

   Written by Chen Guo, chenguo4@ucla.edu.
   Under advisement of Paul Eggert.  */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "command.h"
#include "dgraph.h"
#include "eval.h"
#include "file.h"
#include "frontier.h"
#include "parse.h"
#include "var.h"

#include "parsh.h"


#define PROMPT "$ "

static void *eval_loop (void *);
static void *parse_loop (void *);
static void *reap_loop (void *);
static void print_prompt (void);

static void parsh_init ()
{
#ifdef DEBUG
  dbg = fopen ("debug", "w");
#endif
  file_init ();
  var_init ();
}

int
main (void)
{
  parsh_init ();

  pthread_t eval;
  pthread_t reap;

  /* Initialize the frontier. */
  

  /* Start the loops. */
  pthread_create (&eval, NULL, eval_loop, NULL);  
  pthread_create (&reap, NULL, reap_loop, NULL);
  parse_loop (NULL);

  return 0;
}

/* Eval loop. */
static void *
eval_loop (void *ignore)
{
  struct dg_node *command;

  DBG("Eval loop initiated.\n");
  for (;;)
    {
      command = frontier_run ();

      DBG("EVAL_LOOP: got cmd.\n");
      eval_cmd (command);
    }
  return NULL;
}

/* Parse lopp. */
static void *
parse_loop (void *ignore)
{
  FILE *input = stdin;

  parse_init (input);

  DBG("Parse loop initiated.\n");
  for (;;)
    {
      /* Construct command tree. */
      print_prompt ();
      if (!parse_input (input))
        break;
    }

  /* TODO: Exit the other loops. */

  return NULL;
}

/* Reap loop. */
static void *
reap_loop (void *ignore)
{
  return NULL;
}

/* Print prompt. */
static void
print_prompt (void)
{
  printf (PROMPT);
}
