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

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "dgraph.h"
#include "parsh.h"

#include "parse.h"


/* HUGE TODO: make it so that there's no arbitrary input line limit. */
#define DEFAULT_INBUF_SIZE 1024
#define DEFAULT_TOKEN_SIZE 256

struct buffer
{
  char *buf;                     /* Start of buffer. */
  char *ptr;                     /* Next unused byte. */
  char *lim;                     /* One past last byte. */
  size_t siz;                    /* Maximum size of buffer. */
};

static struct buffer linbuf;
static struct buffer tokbuf;


/* Initialize the parser. */
void parse_init (FILE *input)
{
  /* Initialize input buffer. */
  linbuf.siz = DEFAULT_INBUF_SIZE;
  linbuf.buf = malloc (linbuf.siz * sizeof (char));
  linbuf.ptr = linbuf.buf;
  linbuf.lim = linbuf.buf + linbuf.siz;

  /* Initialize token buffer. */
  tokbuf.siz = DEFAULT_TOKEN_SIZE;
  tokbuf.buf = malloc (tokbuf.siz * sizeof (char));
  tokbuf.ptr = tokbuf.buf;
  tokbuf.lim = tokbuf.buf + tokbuf.siz;

  /* Open input stream. */
  /* fopen (input, 'r'); */
}


/* Read next line of command. */
static void
parse_readline (FILE *input)
{
  if (!fgets (linbuf.buf, linbuf.siz, input))
    abort ();
  else
    linbuf.ptr = linbuf.buf;
}


/* Push a character onto the token buffer. */
static void
putc_tok (char c)
{
  /* TODO: realloc if buffer is full. */
  *tokbuf.ptr++ = c;
}


/* Handle escaped character. */
static void
rmescape (char c)
{

}

/* Break an input line into tokens. Here line refers to syntactic line,
   i.e. not one that is broken by escaped newlines. */
static struct arglist *
parse_token (FILE *input)
{
  struct arglist *args = NULL;
  struct arglist **argsp = &args;
  bool esc = false;
  bool s_quote = false;
  bool d_quote = false;
  size_t toksiz = 0;

  /* Until the end of the line. */
  while (1)
    {
      /* Skip unescaped whitespace. */
      while (isspace (*linbuf.ptr))
        {
          /* TODO: handle escaped newlines. */
          if (*linbuf.ptr == '\n')
            {
              /* Thre entire line has been processed. */
              return args;
            }
          linbuf.ptr++;
        }
    
      /* Get next token. Provided that no quoting is in effect, this can
         be delimited by any character from the following:
           whitespace | & ; < > ( ) $
         TODO: support character delineation by these characters.
         TODO: make robust.
	 TODO: optimize this conditional. */
      while (linbuf.ptr < linbuf.lim
             && !(isspace (*linbuf.ptr) && !(esc | s_quote | d_quote)))
        {
          char c = *linbuf.ptr;
          //DBG("%p, %p, %c:%d\n", linbuf.ptr, linbuf.lim, c, c);
          switch (c)
            {
            case '\\':
              if (!esc)
                esc = true;
              else
                rmescape (c);
              break;

            case '"':
              if (d_quote)
                d_quote = false;
              else if (!s_quote)
                d_quote = true;
              else
                putc_tok (c);
              break;

            case '\'':
              if (s_quote)
                s_quote = false;
              else if (!d_quote)
                s_quote = true;
              else
                putc_tok (c);
              break;

            default:
              putc_tok (c);
            }
          linbuf.ptr++;
        }
      /* We hit the buffer limit, before the end of the line.
         Read in more. */
      if (linbuf.ptr == linbuf.lim)
        {
          parse_readline (input);
          continue;
        }

      /* Put token into arglist. */
      toksiz = tokbuf.ptr - tokbuf.buf;
      if (toksiz)
        {
          /* Allocate a new arglist node. */
          *argsp = malloc (sizeof **argsp);
          (*argsp)->arg = malloc (toksiz * sizeof (char));
          /* Copy the token from the token buffer and reset token buffer. */
          memcpy ((*argsp)->arg, tokbuf.buf, toksiz);
          tokbuf.ptr = tokbuf.buf;
          /* Update variables. */
          toksiz = 0;
          (*argsp)->next = NULL;
          argsp = &(*argsp)->next;
        }
    }
}


/* Print argument list. */
static void
print_args (struct arglist *args)
{
  while (args)
    {
      DBG("Token: %s\n", args->arg);
      args = args->next;
    }
}


/* Parse a command, converting it into a command tree. */
static union command *
parse_command (struct arglist *args)
{
  /* For now,
     1) First arg is command.
     2) Remaining args are command arguments. */
  union command *cmdtree = (union command *) malloc (sizeof (struct ccmd));
  cmdtree->type = COMMAND;
  cmdtree->ccmd.cmd = args->arg;
  cmdtree->ccmd.args = args->next;

  return cmdtree;
}


/* Parse a line. This is the entry point into the parser for the main
   loop. */
union command *
parse_input (FILE *input)
{
  /* Read a line. */
  parse_readline (input);

  /* Break down the line into tokens. */
  struct arglist *args = parse_token (input);
  if (!args)
    {
      return NULL;
    }
  print_args(args);

  /* Recursively process tokens and build command tree. */
  return parse_command (args);
}
