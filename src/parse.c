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
#define KEEP_ARG false
#define FREE_ARG true
#define REFILL_PROMPT "> "

struct buffer
{
  char *buf;                     /* Start of buffer. */
  char *ptr;                     /* Next unused byte. */
  char *lim;                     /* One past last byte. */
  size_t siz;                    /* Maximum size of buffer. */
};

/* Types of statement delimiters. */
#define DELIM_NONE NULL
#define DELIM_THEN "then"
#define DELIM_ELSE "else"
#define DELIM_FI   "fi"
#define DELIM_DO   "do"
#define DELIM_DONE "done"

/* Buffers. */
static struct buffer linbuf;
static struct buffer tokbuf;

/* Token list. */
static struct arglist *toks;

/* Initialize the parser. */
void parse_init (FILE *input_arg)
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

  /* Initial token list is empty */
  toks = NULL;

  /* Open input stream. */
  //input = fopen (input_arg, 'r');
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


/* Handle escaped character.
static void
rmescape (char c)
{

}*/


/* Break an input line into tokens. Here line refers to syntactic line,
   i.e. not one that is broken by escaped newlines. */
static struct arglist *
parse_token (FILE *input)
{
  DBG("PARSE TOKEN\n");

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
          /* The entire command has been processed. */
          if (*linbuf.ptr == '\n')
            return args;
          linbuf.ptr++;
        }

      /* Get next token. */
      while (1)
        {
          char c = *linbuf.ptr;

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
                  printf (REFILL_PROMPT);
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
          DBG("  token: %s, size: %ld\n", (*argsp)->arg, toksiz);
          /* Update variables. */
          tokbuf.ptr = tokbuf.buf;
          toksiz = 0;
          (*argsp)->next = NULL;
          argsp = &(*argsp)->next;
        }
    }
}


/* Print argument list.
static void
print_args (struct arglist *args)
{
  DBG("PRINT ARGS\n");
  while (args)
    {
      DBG("  Token: %s\n", args->arg);
      args = args->next;
    }
}*/


/* Free a TOKEN in the global token list. Set the global token list to point
   at the next token. */
static void
next_tok (bool free_string)
{
  struct arglist *next = toks->next;
  if (free_string)
    free (toks->arg);
  free (toks);
  toks = next;
}


/* Attemp to read expected remainder of statement. */
static bool
refill (FILE *input)
{
  printf (REFILL_PROMPT);
  parse_readline (input);
  if (!ferror (input))
    {
      toks = parse_token (input);
      return true;
    }
  else
    return false;
}


/* Determine type of argument. */
static int
get_arg_type (char *redir)
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
  else if (strcmp (redir, "&") == 0)
    return BACKGND;
  else if (strcmp (redir, ";") == 0)
    return SEMICOLON;
  else
    return NO_REDIR;
}


/* Check if a required delimiter is matched. */
static bool
delim_match (char *delim, FILE *input)
{
  if (delim == DELIM_NONE)
    return true;

  while (!toks)
    refill (input);

  /* TODO: in case of "else", "fi" should also be
     acceptable. */
  DBG("DELIM MATCH: %s  tok: %s\n", delim, toks->arg);
  if (strcmp (toks->arg, delim) == 0)
    return true;
  else
    return false;
}

/* Parse a regular command. */
static struct command *
parse_command (struct command *parent)
{
  struct command *command;

  command = ct_alloc (CT_COMMAND, parent);
  /* First argument is the command itself. */
  command->cmdtree->ccmd.cmdstr = toks->arg;
  next_tok (KEEP_ARG);
  DBG("PARSE COMMAND: CMD: %s, PARENT: %p\n", command->cmdtree->ccmd.cmdstr, parent);
  /* Step through remaining tokens, separating arguments from
     redirections. */
  struct arglist **argp = &command->cmdtree->ccmd.args;
  struct redir **redirp = &command->cmdtree->ccmd.redirs;

  /* Loop over command arguments. */
  while (toks)
    {
      int arg_type = get_arg_type (toks->arg);
      /* If not an operator, append to arguments list. */
      /* TODO: maybe more elegant (and proper) way of ignoring
         ; and & */
      DBG("    ARG: %s\n", toks->arg);
      if (arg_type == NO_REDIR)
        {
          /* Append current argument to command's arguments list. */
          *argp = toks;
          argp = &(*argp)->next;
          toks = toks->next;
          DBG("toks: %p\n", toks);
        }
      else if (arg_type == BACKGND)
        {
          /* Ignore this case. */
          next_tok (FREE_ARG);
        }
      else if (arg_type == SEMICOLON)
        {
          next_tok (FREE_ARG);
          break;
        }
      else
        {
          /* Create new redirection. */
          *redirp = malloc (sizeof **redirp);
          (*redirp)->type = arg_type;
          (*redirp)->next = NULL;
          next_tok (FREE_ARG);

          /* Get the redirection target. */
          (*redirp)->file = toks->arg;
          (*redirp)->fd = 0;
          DBG("Redirection added: %s\n", toks->arg);
          redirp = &(*redirp)->next;
          next_tok (KEEP_ARG);
        }
    }
  /* Delimit the arguments linked list. */
  *argp = NULL;
  return command;
}


/* Return a STRUCT COMMAND containing the parsed command tree. For structured
   statements like () and IF-THEM-ELSE, etc, a deliminator is specified.

   TODO: on encountering a parse error, let user know what the
   error was. */
static struct command *
parse_command_tree (struct command *parent, FILE *input, char *delim,
                    bool free_delim)
{
  DBG("PARSE COMMAND TREE: PARENT %p, DELIM %s\n", parent, delim);
  struct command *command;

  while (!toks)
    {
      if (!refill (input))
        return NULL;
    }

  if (strcmp (toks->arg, "if") == 0 || strcmp (toks->arg, "elif") == 0)
    {
      /* Skip over the if. */
      next_tok (FREE_ARG);

      /* IF statement. */
      command = ct_alloc (CT_IF, parent);
      struct cif *cif = &command->cmdtree->cif;
      cif->cif_cond = parse_command_tree (command, input, DELIM_THEN, true);
      cif->cif_then = parse_command_tree (command, input, DELIM_ELSE, true);
      cif->cif_else = parse_command_tree (command, input, DELIM_FI, true);
    }
  else if (strcmp (toks->arg, "for") == 0)
    {

    }
  else if (strcmp (toks->arg, "while") == 0)
    {
      /* Skip over the while. */
      next_tok (FREE_ARG);

      /* WHILE statement. */
      command = ct_alloc (CT_WHILE, parent);
      struct cwhile *cwhile = &command->cmdtree->cwhile;
      cwhile->cwhile_cond = parse_command_tree (command, input, DELIM_DO, true);
      cwhile->cwhile_body = parse_command_tree (command, input, DELIM_DONE, true);
    }
  else if (strcmp (toks->arg, "until") == 0)
    {
      /* Skip over the until. */
      next_tok (FREE_ARG);

      /* UNTIL statement. */
      command = ct_alloc (CT_UNTIL, parent);
      struct cwhile *cwhile = &command->cmdtree->cwhile;
      cwhile->cwhile_cond = parse_command_tree (command, input, DELIM_DO, true);
      cwhile->cwhile_body = parse_command_tree (command, input, DELIM_DONE, true);
    }
  else if (strcmp (toks->arg, "case") == 0)
    {

    }
  /* Keywords.
     TODO: detect if this is out of place, and print some kind of
     error message. */
  else if (strcmp (toks->arg, "then") == 0
           || strcmp (toks->arg, "else") == 0
           || strcmp (toks->arg, "fi") == 0
           || strcmp (toks->arg, "do") == 0
           || strcmp (toks->arg, "done") == 0)
    {
      return NULL;
    }
  else
    {
      command = parse_command (parent);
    }

  /* End conditions:
     1) No delimiter: nothing remaining except maybe ";"
     2) Delimiter: get delimiter. */
  if (delim == DELIM_NONE && !toks)
    {
      return command;
    }
  else
    {
      /* Get the command after the semicolon. */
      struct command *c2 = parse_command_tree (parent, input, delim, false);
      if (c2)
        {
          struct command *c1 = command;
          command = ct_alloc (CT_SEMICOLON, parent);
          c1->parent = command;
          c2->parent = command;
          command->cmdtree->csemi.cmd1 = c1;
          command->cmdtree->csemi.cmd2 = c2;
          command->children = 2;
        }

      /* At this point, the only left is the delimiter. */
      if (delim_match (delim, input))
        {
          if (free_delim && delim != DELIM_NONE)
            next_tok (FREE_ARG);
          return command;
        }
      else
        {
          fprintf (stderr, "PARSE ERROR.\n");
          return NULL;
        }
    }
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
  DBG("PARSE INPUT\n");

  /* Read a line. */
  parse_readline (input);

  /* Break down the line into tokens. */
  toks = parse_token (input);
  //print_args(toks);
  if (toks)
    {
      /* Recursively process tokens and build command tree. */
      struct command *cmd = parse_command_tree (NULL, input, DELIM_NONE, true);
      if (cmd)
        file_command_process (cmd, FILE_ADD);
    }
  return !feof (input);
}
