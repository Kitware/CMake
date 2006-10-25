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
#include <ctype.h>  /* isalpha */

#include <stdio.h>

/*

Notes:

Make variable replacements open a can of worms.  Sometimes they should
be quoted and sometimes not.  Sometimes their replacement values are
already quoted.

VS variables cause problems.  In order to pass the referenced value
with spaces the reference must be quoted.  If the variable value ends
in a backslash then it will escape the ending quote!  In order to make
the ending backslash appear we need this:

  "$(InputDir)\"

However if there is not a trailing backslash then this will put a
quote in the value so we need:

  "$(InputDir)"

Make variable references are platform specific so we should probably
just NOT quote them and let the listfile author deal with it.

*/

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharIsWhitespace(char c)
{
  return ((c == ' ') || (c == '\t'));
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharNeedsQuotesOnUnix(char c)
{
  return ((c == '\'') || (c == '`') || (c == ';') || (c == '#') ||
          (c == '&') || (c == '$') || (c == '(') || (c == ')'));
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharNeedsQuotes(char c, int isUnix, int flags)
{
  /* On Windows the built-in command shell echo never needs quotes.  */
  if(!isUnix && (flags & kwsysSystem_Shell_Flag_EchoWindows))
    {
    return 0;
    }

  /* On all platforms quotes are needed to preserve whitespace.  */
  if(kwsysSystem_Shell__CharIsWhitespace(c))
    {
    return 1;
    }

  if(isUnix)
    {
    /* On UNIX several special characters need quotes to preserve them.  */
    if(kwsysSystem_Shell__CharNeedsQuotesOnUnix(c))
      {
      return 1;
      }
    }
  else
    {
    /* On Windows single-quotes must be escaped in some make
       environments, such as in mingw32-make.  */
    if(flags & kwsysSystem_Shell_Flag_Make)
      {
      if(c == '\'')
        {
        return 1;
        }
      }
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__CharIsMakeVariableName(char c)
{
  return c && (c == '_' || isalpha(((int)c)));
}

/*--------------------------------------------------------------------------*/
static const char* kwsysSystem_Shell__SkipMakeVariables(const char* c)
{
  while(*c == '$' && *(c+1) == '(')
    {
    const char* skip = c+2;
    while(kwsysSystem_Shell__CharIsMakeVariableName(*skip))
      {
      ++skip;
      }
    if(*skip == ')')
      {
      c = skip+1;
      }
    else
      {
      break;
      }
    }
  return c;
}

/*
Allowing make variable replacements opens a can of worms.  Sometimes
they should be quoted and sometimes not.  Sometimes their replacement
values are already quoted or contain escapes.

Some Visual Studio variables cause problems.  In order to pass the
referenced value with spaces the reference must be quoted.  If the
variable value ends in a backslash then it will escape the ending
quote!  In order to make the ending backslash appear we need this:

  "$(InputDir)\"

However if there is not a trailing backslash then this will put a
quote in the value so we need:

  "$(InputDir)"

This macro decides whether we quote an argument just because it
contains a make variable reference.  This should be replaced with a
flag later when we understand applications of this better.
*/
#define KWSYS_SYSTEM_SHELL_QUOTE_MAKE_VARIABLES 0

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__ArgumentNeedsQuotes(const char* in, int isUnix,
                                                  int flags)
{
  /* Scan the string for characters that require quoting.  */
  const char* c;
  for(c=in; *c; ++c)
    {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if(flags & kwsysSystem_Shell_Flag_AllowMakeVariables)
      {
#if KWSYS_SYSTEM_SHELL_QUOTE_MAKE_VARIABLES
      const char* skip = kwsysSystem_Shell__SkipMakeVariables(c);
      if(skip != c)
        {
        /* We need to quote make variable references to preserve the
           string with contents substituted in its place.  */
        return 1;
        }
#else
      /* Skip over the make variable references if any are present.  */
      c = kwsysSystem_Shell__SkipMakeVariables(c);

      /* Stop if we have reached the end of the string.  */
      if(!*c)
        {
        break;
        }
#endif
      }

    /* Check whether this character needs quotes.  */
    if(kwsysSystem_Shell__CharNeedsQuotes(*c, isUnix, flags))
      {
      return 1;
      }
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
static int kwsysSystem_Shell__GetArgumentSize(const char* in,
                                              int isUnix, int flags)
{
  /* Start with the length of the original argument, plus one for
     either a terminating null or a separating space.  */
  int size = (int)strlen(in) + 1;

  /* String iterator.  */
  const char* c;

  /* Keep track of how many backslashes have been encountered in a row.  */
  int windows_backslashes = 0;

  /* Scan the string for characters that require escaping or quoting.  */
  for(c=in; *c; ++c)
    {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if(flags & kwsysSystem_Shell_Flag_AllowMakeVariables)
      {
      /* Skip over the make variable references if any are present.  */
      c = kwsysSystem_Shell__SkipMakeVariables(c);

      /* Stop if we have reached the end of the string.  */
      if(!*c)
        {
        break;
        }
      }

    /* Check whether this character needs escaping for the shell.  */
    if(isUnix)
      {
      /* On Unix a few special characters need escaping even inside a
         quoted argument.  */
      if(*c == '\\' || *c == '"' || *c == '`' || *c == '$')
        {
        /* This character needs a backslash to escape it.  */
        ++size;
        }
      }
    else if(flags & kwsysSystem_Shell_Flag_EchoWindows)
      {
      /* On Windows the built-in command shell echo never needs escaping.  */
      }
    else
      {
      /* On Windows only backslashes and double-quotes need escaping.  */
      if(*c == '\\')
        {
        /* Found a backslash.  It may need to be escaped later.  */
        ++windows_backslashes;
        }
      else if(*c == '"')
        {
        /* Found a double-quote.  We need to escape it and all
           immediately preceding backslashes.  */
        size += windows_backslashes + 1;
        windows_backslashes = 0;
        }
      else
        {
        /* Found another character.  This eliminates the possibility
           that any immediately preceding backslashes will be
           escaped.  */
        windows_backslashes = 0;
        }
      }

    /* Check whether this character needs escaping for a make tool.  */
    if(*c == '$')
      {
      if(flags & kwsysSystem_Shell_Flag_Make)
        {
        /* In Makefiles a dollar is written $$ so we need one extra
           character.  */
        ++size;
        }
      else if(flags & kwsysSystem_Shell_Flag_VSIDE)
        {
        /* In a VS IDE a dollar is written "$" so we need two extra
           characters.  */
        size += 2;
        }
      }
    else if(*c == '#')
      {
      if((flags & kwsysSystem_Shell_Flag_Make) &&
         (flags & kwsysSystem_Shell_Flag_WatcomWMake))
        {
        /* In Watcom WMake makefiles a pound is written $# so we need
           one extra character.  */
        ++size;
        }
      }
    }

  /* Check whether the argument needs surrounding quotes.  */
  if(kwsysSystem_Shell__ArgumentNeedsQuotes(in, isUnix, flags))
    {
    /* Surrounding quotes are needed.  Allocate space for them.  */
    size += 2;

    /* We must escape all ending backslashes when quoting on windows.  */
    size += windows_backslashes;
    }

  return size;
}

/*--------------------------------------------------------------------------*/
static char* kwsysSystem_Shell__GetArgument(const char* in, char* out,
                                            int isUnix, int flags)
{
  /* String iterator.  */
  const char* c;

  /* Keep track of how many backslashes have been encountered in a row.  */
  int windows_backslashes = 0;

  /* Whether the argument must be quoted.  */
  int needQuotes = kwsysSystem_Shell__ArgumentNeedsQuotes(in, isUnix, flags);
  if(needQuotes)
    {
    /* Add the opening quote for this argument.  */
    *out++ = '"';
    }

  /* Scan the string for characters that require escaping or quoting.  */
  for(c=in; *c; ++c)
    {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if(flags & kwsysSystem_Shell_Flag_AllowMakeVariables)
      {
      const char* skip = kwsysSystem_Shell__SkipMakeVariables(c);
      if(skip != c)
        {
        /* Copy to the end of the make variable references.  */
        while(c != skip)
          {
          *out++ = *c++;
          }

        /* Stop if we have reached the end of the string.  */
        if(!*c)
          {
          break;
          }
        }
      }

    /* Check whether this character needs escaping for the shell.  */
    if(isUnix)
      {
      /* On Unix a few special characters need escaping even inside a
         quoted argument.  */
      if(*c == '\\' || *c == '"' || *c == '`' || *c == '$')
        {
        /* This character needs a backslash to escape it.  */
        *out++ = '\\';
        }
      }
    else if(flags & kwsysSystem_Shell_Flag_EchoWindows)
      {
      /* On Windows the built-in command shell echo never needs escaping.  */
      }
    else
      {
      /* On Windows only backslashes and double-quotes need escaping.  */
      if(*c == '\\')
        {
        /* Found a backslash.  It may need to be escaped later.  */
        ++windows_backslashes;
        }
      else if(*c == '"')
        {
        /* Found a double-quote.  Escape all immediately preceding
           backslashes.  */
        while(windows_backslashes > 0)
          {
          --windows_backslashes;
          *out++ = '\\';
          }

        /* Add the backslash to escape the double-quote.  */
        *out++ = '\\';
        }
      else
        {
        /* We encountered a normal character.  This eliminates any
           escaping needed for preceding backslashes.  */
        windows_backslashes = 0;
        }
      }

    /* Check whether this character needs escaping for a make tool.  */
    if(*c == '$')
      {
      if(flags & kwsysSystem_Shell_Flag_Make)
        {
        /* In Makefiles a dollar is written $$.  The make tool will
           replace it with just $ before passing it to the shell.  */
        *out++ = '$';
        *out++ = '$';
        }
      else if(flags & kwsysSystem_Shell_Flag_VSIDE)
        {
        /* In a VS IDE a dollar is written "$".  If this is written in
           an un-quoted argument it starts a quoted segment, inserts
           the $ and ends the segment.  If it is written in a quoted
           argument it ends quoting, inserts the $ and restarts
           quoting.  Either way the $ is isolated from surrounding
           text to avoid looking like a variable reference.  */
        *out++ = '"';
        *out++ = '$';
        *out++ = '"';
        }
      else
        {
        /* Otherwise a dollar is written just $. */
        *out++ = '$';
        }
      }
    else if(*c == '#')
      {
      if((flags & kwsysSystem_Shell_Flag_Make) &&
         (flags & kwsysSystem_Shell_Flag_WatcomWMake))
        {
        /* In Watcom WMake makefiles a pound is written $#.  The make
           tool will replace it with just # before passing it to the
           shell.  */
        *out++ = '$';
        *out++ = '#';
        }
      else
        {
        /* Otherwise a pound is written just #. */
        *out++ = '#';
        }
      }
    else
      {
      /* Store this character.  */
      *out++ = *c;
      }
    }

  if(needQuotes)
    {
    /* Add enough backslashes to escape any trailing ones.  */
    while(windows_backslashes > 0)
      {
      --windows_backslashes;
      *out++ = '\\';
      }

    /* Add the closing quote for this argument.  */
    *out++ = '"';
    }

  /* Store a terminating null without incrementing.  */
  *out = 0;

  return out;
}

/*--------------------------------------------------------------------------*/
char* kwsysSystem_Shell_GetArgumentForWindows(const char* in,
                                              char* out,
                                              int flags)
{
  return kwsysSystem_Shell__GetArgument(in, out, 0, flags);
}

/*--------------------------------------------------------------------------*/
char* kwsysSystem_Shell_GetArgumentForUnix(const char* in,
                                           char* out,
                                           int flags)
{
  return kwsysSystem_Shell__GetArgument(in, out, 1, flags);
}

/*--------------------------------------------------------------------------*/
int kwsysSystem_Shell_GetArgumentSizeForWindows(const char* in, int flags)
{
  return kwsysSystem_Shell__GetArgumentSize(in, 0, flags);
}

/*--------------------------------------------------------------------------*/
int kwsysSystem_Shell_GetArgumentSizeForUnix(const char* in, int flags)
{
  return kwsysSystem_Shell__GetArgumentSize(in, 1, flags);
}
