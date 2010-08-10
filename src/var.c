/* Shell variables.
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

#include <pthreads.h>

#include "var.h"

static pthread_mutex_t var_lock;

#define VAR_LOCK {pthread_mutex_lock (&var_lock;}
#define VAR_UNLOCK {pthread_mutex_unlock (&var_lock;}

/* Hash table based off of Dash's implementation. */
#define VTABSIZE 39
struct var *vartab[VTABSIZE];

const char default_path[] =
  "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin";

void
var_init (void)
{
  pthread_mutex_init (&var_lock, NULL);
  VAR_LOCK;

  /* Initialize the hash table. */
  /* TODO: write hash function. PWD??? */

  VAR_UNLOCK;
}

