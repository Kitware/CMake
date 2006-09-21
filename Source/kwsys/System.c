/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(System.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "System.h.in"
#endif

#include <string.h> /* strlen */

/*--------------------------------------------------------------------------*/
static int kwsysSystemWindowsShellArgumentNeedsEscape(const char* in)
{
  /* Scan the string for characters that need escaping.  Note that
     single quotes seem to need escaping for some windows shell
     environments (mingw32-make shell for example).  Single quotes do
     not actually need backslash escapes but must be in a
     double-quoted argument.  */
  const char* c;
  for(c=in; *c; ++c)
    {
    if(*c == ' ' || *c == '\t' || *c == '"' || *c == '\'')
      {
      return 1;
      }
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
int kwsysSystem_Windows_ShellArgumentSize(const char* in)
{
  /* Start with the length of the original argument, plus one for
     either a terminating null or a separating space.  */
  int length = (int)strlen(in) + 1;

  /* String iterator.  */
  const char* c;

  /* Keep track of how many backslashes have been encountered in a row.  */
  int backslashes = 0;

  /* If nothing needs escaping, we do not need any extra length. */
  if(!kwsysSystemWindowsShellArgumentNeedsEscape(in))
    {
    return length;
    }

  /* Add 2 for double quotes since spaces are present.  */
  length += 2;

  /* Scan the string to find characters that need escaping.  */
  for(c=in; *c; ++c)
    {
    if(*c == '\\')
      {
      /* Found a backslash.  It may need to be escaped later.  */
      ++backslashes;
      }
    else if(*c == '"')
      {
      /* Found a double-quote.  We need to escape it and all
         immediately preceding backslashes.  */
      length += backslashes + 1;
      backslashes = 0;
      }
    else
      {
      /* Found another character.  This eliminates the possibility
         that any immediately preceding backslashes will be
         escaped.  */
      backslashes = 0;
      }
    }

  /* We need to escape all ending backslashes. */
  length += backslashes;

  return length;
}

/*--------------------------------------------------------------------------*/
char* kwsysSystem_Windows_ShellArgument(const char* in, char* out)
{
  /* String iterator.  */
  const char* c;

  /* Keep track of how many backslashes have been encountered in a row.  */
  int backslashes = 0;

  /* If nothing needs escaping, we can pass the argument verbatim. */
  if(!kwsysSystemWindowsShellArgumentNeedsEscape(in))
    {
    /* Just copy the string.  */
    for(c=in; *c; ++c)
      {
      *out++ = *c;
      }

    /* Store a terminating null without incrementing.  */
    *out = 0;
    return out;
    }

  /* Add the opening double-quote for this argument.  */
  *out++ = '"';

  /* Add the characters of the argument, possibly escaping them.  */
  for(c=in; *c; ++c)
    {
    if(*c == '\\')
      {
      /* Found a backslash.  It may need to be escaped later.  */
      ++backslashes;
      *out++ = '\\';
      }
    else if(*c == '"')
      {
      /* Add enough backslashes to escape any that preceded the
         double-quote.  */
      while(backslashes > 0)
        {
        --backslashes;
        *out++ = '\\';
        }

      /* Add the backslash to escape the double-quote.  */
      *out++ = '\\';

      /* Add the double-quote itself.  */
      *out++ = '"';
      }
    else
      {
      /* We encountered a normal character.  This eliminates any
         escaping needed for preceding backslashes.  Add the
         character.  */
      backslashes = 0;
      *out++ = *c;
      }
    }

  /* Add enough backslashes to escape any trailing ones.  */
  while(backslashes > 0)
    {
    --backslashes;
    *out++ = '\\';
    }

  /* Add the closing double-quote for this argument.  */
  *out++ = '"';

  /* Store a terminating null without incrementing.  */
  *out = 0;

  return out;
}
