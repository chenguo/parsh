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

   Written by Chen Guo, chenguo4@ucla.edu.
   Under advisement of Paul Eggert.  */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "file.h"
#include "parsh.h"

#include "parse.h"

/* HUGE TODO: make it so that there's no arbitrary input line limit. */
#define DEFAULT_INBUF_SIZE 9
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
  /* Realloc if buffer is full. */
  if (tokbuf.ptr == tokbuf.lim)
    {
      /* Increase buffer size by a factor of two. */
      size_t siz = tokbuf.siz * 2;
      tokbuf.buf = realloc (tokbuf.buf, siz);

      if (NULL == tokbuf.buf)
        abort ();

      tokbuf.ptr = tokbuf.buf + tokbuf.siz;
      tokbuf.lim = tokbuf.buf + siz;
      tokbuf.siz = siz;
    }

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
  char opc = '\0';
  bool esc = false;
  bool s_quote = false;
  bool d_quote = false;
  bool op = false;
  size_t toksiz = 0;

  /* Until the end of the line. */
  while (1)
    {
      /* Skip unescaped whitespace. */
      while (isspace (*linbuf.ptr))
        {
          /* TODO: handle escaped newlines. */
          /* The entire line has been processed. */
          if (*linbuf.ptr == '\n')
            return args;
          linbuf.ptr++;
        }
    
      /* Get next token. */
      while (1)
        {
          char c = *linbuf.ptr;
          //DBG("Buf left %ld, %c:%d, e:s:d:o %d%d%d%d\n",
          //    linbuf.lim - linbuf.ptr, c, c, esc, s_quote,
          //    d_quote, op);

          /* TODO: verify that this only happens at last byte
             of buffer. I.e., could it happen that there's
             more useful bytes after the '\0'? */
          if (c == '\0')
            {
              parse_readline (input);
              continue;
            }

          if (!(esc | s_quote | d_quote))
            {
              /* If not quoted: */
              switch (c)
                {
                  /* Operators. */
                  /* TODO: file descriptors, such as 2> */
                case '&':
                case '|':
                case ';':
                case '<':
                case '>':
                  if (tokbuf.ptr - tokbuf.buf == 0)
                    {
                      /* First character of a new operator. */
                      putc_tok (c);
                      op = true;
                      opc = c;
                      break;
                    }
                  else
                    {
                      /* TODO: 3 character <<- operator. */
                      if (op)
                        {
                          if ((c == '&' && (opc == '&' || opc == '<'
                                            || opc == '>'))
                              || (c == '|' && (opc == '|' || opc == '>'))
                              || (c == ';' && opc == ';')
                              || (c == '<' && opc == '<')
                              || (c == '>' && (opc == '>' || opc == '<')))
                            {
                              /* Unset operator flag. */
                              op = false;
                              putc_tok (c);
                              linbuf.ptr++;
                            }
                          else
                            ; /* TODO: Unrecognized operator. */
                        }
                      /* Else fall through: delineate previous token. */
		      op = false;
                      goto tokend;
                    }

                /* Quoting characters. */
                case '\\':
                  esc = true;
                  if (op)
                    goto tokend;
                  break;

                case '\'':
                  s_quote = true;
                  if (op)
                    goto tokend;
                  break;

                case '"':
                  d_quote = true;
                  if (op)
                    goto tokend;
                  break;

                default:
                  if (isspace (c) || op)
                    {
                      op = false;
                      goto tokend;
                    }
                  else
                    putc_tok (c);
                }
            }
          else
            {
              /* Quoted. */
              switch (c)
                {
                case '\'':
                  if (s_quote && !esc)
                    s_quote = false;
                  else
                    putc_tok (c);
                  break;

                case '"':
                  if (d_quote && !esc)
                    d_quote = false;
                  else
                    putc_tok (c);
                  break;

                case '\n':
                  printf ("> ");
                  parse_readline (input);
                  if (!esc)
                    putc_tok (c);
                  esc = (esc)? !esc : esc;
                  continue;

                default:
                  putc_tok (c);
                  esc = (esc)? !esc : esc;
                }
            }
          linbuf.ptr++;
        }
    tokend:
      /* Put token into arglist. */
      toksiz = tokbuf.ptr - tokbuf.buf;
      if (toksiz)
        {
          /* Allocate a new arglist node, copy token. */
          *argsp = malloc (sizeof **argsp);
          (*argsp)->arg = strncpy_nul (tokbuf.buf, toksiz);
          DBG("PARSE TOKEN: token: %s, size: %d\n", (*argsp)->arg, toksiz);
          /* Update variables. */
          tokbuf.ptr = tokbuf.buf;
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


/* Determine type of redirection. */
static int
redir_type (char *redir)
{
  if (strcmp (redir, "<") == 0)
    return FILE_IN;
  else if (strcmp (redir, ">") == 0)
    return FILE_OUT;
  else if (strcmp (redir, ">|") == 0)
    return FILE_CLOBBER;
  else if (strcmp (redir, ">>") == 0)
    return FILE_APPEND;
  /* TODO: DUPE_IN, DUPE_OUT, support syntax like "2<" */
  else
    return NO_REDIR;
}

/* Parse a command, converting it into a command tree. */
static union cmdtree *
parse_command (struct arglist *args)
{
  union cmdtree *cmdtree;

  if (strcmp (args->arg, "if") == 0)
    {

    }
  else if (strcmp (args->arg, "for") == 0)
    {

    }
  else if (strcmp (args->arg, "while") == 0)
    {

    }
  else if (strcmp (args->arg, "until") == 0)
    {

    }
  else if (strcmp (args->arg, "case") == 0)
    {

    }
  else
    {
      /* Regular command. */
      cmdtree = (union cmdtree *) malloc (sizeof (struct ccmd));
      cmdtree->type = COMMAND;
      /* First argument is the command itself. */
      cmdtree->ccmd.cmdstr = args->arg;
      /* Step through remaining tokens, separating arguments from
         redirections. */
      struct arglist **argp = &cmdtree->ccmd.args;
      struct redir **redirp = &cmdtree->ccmd.redirs;
      /* Loop over command arguments. */
      struct arglist *free_arg = args;
      args = args->next;
      free (free_arg);
      while (args)
        {
          int arg_type = redir_type (args->arg);
          /* If not an operator, append to arguments list. */
          if (arg_type == NO_REDIR)
            {
              /* Append current argument to command's arguments list. */
              *argp = args;
              argp = &(*argp)->next;
              args = args->next;
            }
          else
            {
              /* Create new redirection. */
              *redirp = malloc (sizeof **redirp);
              (*redirp)->type = arg_type;
              (*redirp)->next = NULL;

              /* Free redirection operator resources. */
              free_arg = args;
              args = args->next;
              free (free_arg->arg);
              free (free_arg);

              /* Get the redirection target. */
              (*redirp)->file = args->arg;
              (*redirp)->fd = 0;
              DBG("Redirection added: %s\n", args->arg);

              /* Free the arglist struct. */
              free_arg = args;
              args = args->next;
              free (free_arg);
              redirp = &(*redirp)->next;

            }
        }
      /* Delimit the arguments linked list. */
      *argp = NULL;
    }
  return cmdtree;
}


/* Parse a line. This is the entry point into the parser for the main
   loop.

   This function first reads in a line. Then it breaks the line down
   into individual tokens, and finally creates a command tree out of 
   those tokens, inserting each tree into the graph.

   This function will return true until EOF is reached. */
bool
parse_input (FILE *input)
{
  /* Read a line. */
  parse_readline (input);

  /* Break down the line into tokens. */
  struct arglist *args = parse_token (input);
  if (!args)
    return true;
  print_args(args);

  /* Recursively process tokens and build command tree. */
  union cmdtree *cmdtree = parse_command (args);
  if (cmdtree)
    {
      file_add_command (cmdtree);
      return true;
    }
  return false;
}
