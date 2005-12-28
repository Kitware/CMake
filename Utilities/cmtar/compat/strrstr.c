/*
**  Copyright 1998-2002 University of Illinois Board of Trustees
**  Copyright 1998-2002 Mark D. Roth
**  All rights reserved.
**
**  strrstr.c - strrstr() function for compatibility library
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <stdio.h>
#include <sys/types.h>

#include <string.h>


/*
** find the last occurrance of find in string
*/
char *
strrstr(char *string, char *find)
{
  size_t stringlen, findlen;
  char *cp;

  findlen = strlen(find);
  stringlen = strlen(string);
  if (findlen > stringlen)
    return NULL;

  for (cp = string + stringlen - findlen; cp >= string; cp--)
    if (strncmp(cp, find, findlen) == 0)
      return cp;

  return NULL;
}


