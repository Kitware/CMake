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
/*-------------------------------------------------------------------------
  Portions of this source have been derived from makefdep90 version 2.6.2,

   Copyright (C) 2000,2001 Erik Edelmann <eedelman@beam.helsinki.fi>.

  The code was originally distributed under the GPL but permission
  from the copyright holder has been obtained to distribute this
  derived work under the CMake license.
-------------------------------------------------------------------------*/

/*

This file must be translated to C and modified to build everywhere.

Run flex like this:

  flex --prefix=cmDependsFortran_yy --header-file=cmDependsFortranLexer.h -ocmDependsFortranLexer.cxx cmDependsFortranLexer.in.l

Modify cmDependsFortranLexer.cxx:
  - remove TABs
  - remove "yyscanner" argument from these methods:
      yy_fatal_error, yyalloc, yyrealloc, yyfree

Modify cmDependsFortranLexer.h:
  - remove TABs
  - remove the yy_init_globals function
  - remove the block that includes unistd.h
  - remove #line directives (avoids bogus warning on old Sun)

*/

#define cmDependsFortranLexer_cxx
#include "cmDependsFortranParser.h" /* Interface to parser object.  */

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
#define ECHO

/* Replace the lexer input function.  */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
  { result = cmDependsFortranParser_Input(yyextra, buf, max_size); }

/* Provide isatty on Windows.  */
#if defined( _WIN32 ) && !defined( __CYGWIN__ )
# include <io.h>
# if defined( _MSC_VER )
#  define isatty _isatty
# endif
# define YY_NO_UNISTD_H 1
#endif

/* Include the set of tokens from the parser.  */
#include "cmDependsFortranParserTokens.h"

/*--------------------------------------------------------------------------*/
%}

%option reentrant
%option noyywrap
%pointer

%s free_fmt fixed_fmt
%x str_sq str_dq

%%

\"              {
  cmDependsFortranParser_StringStart(yyextra);
  BEGIN(str_dq);
}

'               {
  cmDependsFortranParser_StringStart(yyextra);
  BEGIN(str_sq);
}

<str_dq>\" |
<str_sq>'  {
  yylvalp->string = strdup(cmDependsFortranParser_StringEnd(yyextra));
  return STRING;
}

<str_dq,str_sq>&[ \t]*\n |
<str_dq,str_sq>&[ \t]*\n[ \t]*&  /* Ignore (continued strings, free fmt) */

<fixed_fmt,str_dq,str_sq>\n[ ]{5}[^ \t\n] /*Ignore (cont. strings, fixed fmt) */

<str_dq,str_sq>\n {
  unput ('\n');
  BEGIN(INITIAL);
  return UNTERMINATED_STRING;
}

<str_sq,str_dq>. {
  cmDependsFortranParser_StringAppend(yyextra, yytext[0]);
}

!.*\n                   { return EOSTMT; } /* Treat comments like */
<fixed_fmt>^[cC*dD].*\n { return EOSTMT; } /* empty lines */

#[ \t]*include  { return CPP_INCLUDE; }
\$[ \t]*include { return F90PPR_INCLUDE; }
\?\?[ \t]*include { return COCO_INCLUDE; }
INCLUDE         { return F_INCLUDE; }
USE             { return USE; }

END" "*INTERFACE {
  cmDependsFortranParser_SetInInterface(yyextra, 0);
}
INTERFACE        {
  cmDependsFortranParser_SetInInterface(yyextra, 1);
}

END" "*MODULE   /* Ignore */
MODULE          {
 if(!cmDependsFortranParser_GetInInterface(yyextra))
   {
   return MODULE;
   }
}

#[ \t]*define   { return CPP_DEFINE; }
\$[ \t]*DEFINE   { return F90PPR_DEFINE; }

#[ \t]*undef    { return CPP_UNDEF; }
\$[ \t]*UNDEF   { return F90PPR_UNDEF; }

#[ \t]*ifdef    { return CPP_IFDEF; }
#[ \t]*ifndef   { return CPP_IFNDEF; }
#[ \t]*if       { return CPP_IF; }
#[ \t]*elif     { return CPP_ELIF; }
#[ \t]*else     { return CPP_ELSE; }
#[ \t]*endif    { return CPP_ENDIF; }

$[ \t]*ifdef    { return F90PPR_IFDEF; }
$[ \t]*ifndef   { return F90PPR_IFNDEF; }
$[ \t]*if       { return F90PPR_IF; }
$[ \t]*elif     { return F90PPR_ELIF; }
$[ \t]*else     { return F90PPR_ELSE; }
$[ \t]*endif    { return F90PPR_ENDIF; }

&[ \t]*\n |
&[ \t]*\n[ \t]*& /* Ignore */


[^ \t\n\r;,!'""']+ { yylvalp->string = strdup(yytext); return WORD; }

;|\n          { return EOSTMT; }

[ \t\r,]         /* Ignore */

.               { return *yytext; }

<<EOF>> {
 if(!cmDependsFortranParser_FilePop(yyextra))
   {
   return YY_NULL;
   }
}

%%

/*--------------------------------------------------------------------------*/
YY_BUFFER_STATE cmDependsFortranLexer_GetCurrentBuffer(yyscan_t yyscanner)
{
  /* Hack into the internal flex-generated scanner to get the buffer.  */
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
  return YY_CURRENT_BUFFER;
}
