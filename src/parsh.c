/* Common fuctions.
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

#include <stdlib.h>
#include <string.h>

/* Make a NUL delimited copy. Passed in STRLEN does not include NUL byte. */
char *
strncpy_nul (const char *s2, size_t strlen)
{
  char *s1 = malloc (strlen + 1);
  strncpy (s1, s2, strlen);
  s1[strlen] = '\0';
  return s1;
}
