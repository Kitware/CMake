%{
/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/*

This file must be translated to C and modified to build everywhere.

Run flex like this:

  flex --prefix=cmCommandArgument_yy --header-file=cmCommandArgumentLexer.h -ocmCommandArgumentLexer.cxx cmCommandArgumentLexer.in.l

Modify cmCommandArgumentLexer.cxx:
  - remove TABs
  - remove "yyscanner" argument from these methods:
      yy_fatal_error, yyalloc, yyrealloc, yyfree
  - remove all YY_BREAK lines occurring right after return statements
  - change while ( 1 ) to for(;;)

Modify cmCommandArgumentLexer.h:
  - remove TABs
  - remove the yy_init_globals function
  - remove the block that includes unistd.h
  - remove #line directives (avoids bogus warning on old Sun)

*/

#include "cmCommandArgumentParserHelper.h"

/* Disable some warnings.  */
#if defined(_MSC_VER)
# pragma warning ( disable : 4127 )
# pragma warning ( disable : 4131 )
# pragma warning ( disable : 4244 )
# pragma warning ( disable : 4251 )
# pragma warning ( disable : 4267 )
# pragma warning ( disable : 4305 )
# pragma warning ( disable : 4309 )
# pragma warning ( disable : 4706 )
# pragma warning ( disable : 4786 )
#endif

/* Disable features we do not need. */
#define YY_NEVER_INTERACTIVE 1
#undef ECHO /* SGI termios defines this differently. */
#define ECHO

/* Replace the lexer input function.  */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
  { result = yyextra->LexInput(buf, max_size); }

/* Include the set of tokens from the parser.  */
#include "cmCommandArgumentParserTokens.h"


#if defined( _WIN32 ) && !defined( __CYGWIN__ )
/* Handle Windows properly */
#  include <io.h>
#  if defined( _MSC_VER )
#    define isatty _isatty
#  endif
#  define YY_NO_UNISTD_H 1
#endif

/*--------------------------------------------------------------------------*/
%}

%option reentrant
%option noyywrap
%pointer

%%

"$[A-Za-z0-9_]+{" { 
  yyextra->AllocateParserType(yylvalp, yytext+1, strlen(yytext)-2); 
  return cal_NCURLY; 
} 

"${" {
  return cal_DCURLY;
}

"}" {
  return cal_RCURLY;
}

"@" {
  return cal_AT;
}

[A-Za-z0-9_]+ { 
  yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
  return cal_NAME; 
}

([^A-Za-z0-9_$}\\@]+|\.) { 
  yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
  return cal_SYMBOL; 
}

%%
