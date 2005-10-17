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

  flex --prefix=cmExpr_yy --header-file=cmExprLexer.h -ocmExprLexer.cxx cmExprLexer.in.l

Modify cmExprLexer.cxx:
  - remove TABs
  - remove "yyscanner" argument from these methods:
      yy_fatal_error, yyalloc, yyrealloc, yyfree
  - remove all YY_BREAK lines occurring right after return statements
  - change while ( 1 ) to for(;;)

Modify cmExprLexer.h:
  - remove TABs
  - remove the yy_init_globals function
  - remove the block that includes unistd.h
  - remove #line directives (avoids bogus warning on old Sun)

*/

#include "cmExprParserHelper.h"

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
#include "cmExprParserTokens.h"


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

[0-9][0-9]* { yylvalp->Number = atoi(yytext); return exp_NUMBER; } 

"+" { return exp_PLUS; }
"-" { return exp_MINUS; } 
"*" { return exp_TIMES; } 
"/" { return exp_DIVIDE; } 
"%" { return exp_MOD; } 
"\|" { return exp_OR; } 
"&" { return exp_AND; } 
"^" { return exp_XOR; } 
"~" { return exp_NOT; } 
"<<" { return exp_SHIFTLEFT; } 
">>" { return exp_SHIFTRIGHT; } 
"(" { return exp_OPENPARENT; }
")" { return exp_CLOSEPARENT; }

%%
