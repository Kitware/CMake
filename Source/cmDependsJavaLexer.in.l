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

  flex --prefix=cmDependsJava_yy --header-file=cmDependsJavaLexer.h -ocmDependsJavaLexer.cxx cmDependsJavaLexer.in.l

Modify cmDependsJavaLexer.c:
  - remove TABs
  - remove "yyscanner" argument from these methods:
      yy_fatal_error, yyalloc, yyrealloc, yyfree
  - remove all YY_BREAK lines occurring right after return statements

Modify cmDependsJavaLexer.h:
  - remove TABs
  - remove the yy_init_globals function
  - remove the block that includes unistd.h

*/

#include "cmDependsJavaParserHelper.h"

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
  { result = yyextra->LexInput(buf, max_size); }

/* Include the set of tokens from the parser.  */
#include "cmDependsJavaParserTokens.h"


#if defined( _WIN32 ) && !defined( __CYGWIN__ )
/* Handle Windows properly */
#  include <io.h>
#  define isatty _isatty
#  define YY_NO_UNISTD_H 1
#endif

#define KEYWORD yylvalp->str = 0
#define SYMBOL yylvalp->str = 0
#define PRIMITIVE  yylvalp->str = 0

/*--------------------------------------------------------------------------*/
%}

%option reentrant
%option noyywrap
%pointer

%x comment
%x string

%%
"/*" { BEGIN(comment); }
<comment>"*/" { BEGIN(INITIAL); }
<comment>.|\n {}

\" { BEGIN(string); }
<string>\" { BEGIN(INITIAL); return STRINGLITERAL; }
<string>. {}

abstract { KEYWORD; return ABSTRACT; }
assert { KEYWORD; return ASSERT; }
boolean { KEYWORD; return BOOLEAN_TYPE; }
break { KEYWORD; return BREAK; }
byte { KEYWORD; return BYTE_TYPE; }
case { KEYWORD; return CASE; }
catch { KEYWORD; return CATCH; }
char { KEYWORD; return CHAR_TYPE; }
class { KEYWORD; return CLASS; }
continue { KEYWORD; return CONTINUE; }
default { KEYWORD; return DEFAULT; }
do { KEYWORD; return DO; }
double { KEYWORD; return DOUBLE_TYPE; }
else { KEYWORD; return ELSE; }
extends { KEYWORD; return EXTENDS; }
final { KEYWORD; return FINAL; }
finally { KEYWORD; return FINALLY; }
float { KEYWORD; return FLOAT_TYPE; }
for { KEYWORD; return FOR; }
if { KEYWORD; return IF; }
implements { KEYWORD; return IMPLEMENTS; }
import { KEYWORD; return IMPORT; }
instanceof { KEYWORD; return INSTANCEOF; }
int { KEYWORD; return INT_TYPE; }
interface { KEYWORD; return INTERFACE; }
long { KEYWORD; return LONG_TYPE; }
native { KEYWORD; return NATIVE; }
new { KEYWORD; return NEW; }
package { KEYWORD; return PACKAGE; }
private { KEYWORD; return PRIVATE; }
protected { KEYWORD; return PROTECTED; }
public { KEYWORD; return PUBLIC; }
return { KEYWORD; return RETURN; }
short { KEYWORD; return SHORT_TYPE; }
static { KEYWORD; return STATIC; }
strictfp { KEYWORD; return STRICTFP; }
super { KEYWORD; return SUPER; }
switch { KEYWORD; return SWITCH; }
synchronized { KEYWORD; return SYNCHRONIZED; }
this { KEYWORD; return THIS; }
throw { KEYWORD; return THROW; }
throws { KEYWORD; return THROWS; }
transient { KEYWORD; return TRANSIENT; }
try { KEYWORD; return TRY; }
void { KEYWORD; return VOID; }
volatile { KEYWORD; return VOLATILE; }
while { KEYWORD; return WHILE; }

(true|false) { PRIMITIVE; return BOOLEANLITERAL; }
\'([^\\]|\\.|\\u[0-9a-fA-F]*|\\[0-7]*)\' { PRIMITIVE; return CHARACTERLITERAL; }
(0|[0-9]+)[lL]? { PRIMITIVE; return DECIMALINTEGERLITERAL; }
([0-9]+\.[0-9]*|\.[0-9]+|[0-9]+)([eE][+\-]?[0-9]+)?[fFdD]? { PRIMITIVE; return FLOATINGPOINTLITERAL; }
0[xX][0-9a-fA-F]+[lL]? { PRIMITIVE; return HEXINTEGERLITERAL; }
null { PRIMITIVE; return NULLLITERAL; }

"&" { SYMBOL; return AND; }
"&&" { SYMBOL; return ANDAND; }
"&=" { SYMBOL; return ANDEQUALS; }
"\]" { SYMBOL; return BRACKETEND; }
"\[" { SYMBOL; return BRACKETSTART; }
"\^" { SYMBOL; return CARROT; }
"\^=" { SYMBOL; return CARROTEQUALS; }
":" { SYMBOL; return COLON; }
"," { SYMBOL; return COMMA; }
"}" { SYMBOL; return CURLYEND; }
"{" { SYMBOL; return CURLYSTART; }
"/" { SYMBOL; return DIVIDE; }
"/=" { SYMBOL; return DIVIDEEQUALS; }
"\$" { SYMBOL; return DOLLAR; }
"\." { SYMBOL; return DOT; }
"=" { SYMBOL; return EQUALS; }
"==" { SYMBOL; return EQUALSEQUALS; }
"\!" { SYMBOL; return EXCLAMATION; }
"\!=" { SYMBOL; return EXCLAMATIONEQUALS; }
">" { SYMBOL; return GREATER; }
">=" { SYMBOL; return GTEQUALS; }
">>" { SYMBOL; return GTGT; }
">>=" { SYMBOL; return GTGTEQUALS; }
">>>" { SYMBOL; return GTGTGT; }
">>>=" { SYMBOL; return GTGTGTEQUALS; }
"<<=" { SYMBOL; return LESLESEQUALS; }
"<" { SYMBOL; return LESSTHAN; }
"<=" { SYMBOL; return LTEQUALS; }
"<<" { SYMBOL; return LTLT; }
"-" { SYMBOL; return MINUS; }
"-=" { SYMBOL; return MINUSEQUALS; }
"--" { SYMBOL; return MINUSMINUS; }
"\)" { SYMBOL; return PAREEND; }
"\(" { SYMBOL; return PARESTART; }
"%" { SYMBOL; return PERCENT; }
"%=" { SYMBOL; return PERCENTEQUALS; }
"\|" { SYMBOL; return PIPE; }
"\|=" { SYMBOL; return PIPEEQUALS; }
"\|\|" { SYMBOL; return PIPEPIPE; }
"\+" { SYMBOL; return PLUS; }
"\+=" { SYMBOL; return PLUSEQUALS; }
"\+\+" { SYMBOL; return PLUSPLUS; }
"\?" { SYMBOL; return QUESTION; }
";" { SYMBOL; return SEMICOL; }
"\~" { SYMBOL; return TILDE; }
"\*" { SYMBOL; return TIMES; }
"\*=" { SYMBOL; return TIMESEQUALS; }

[a-z_A-Z][a-z_0-9A-Z]* { 
  yyextra->AllocateParserType(yylvalp, yytext, strlen(yytext)); 
  return NAME; 
}

\/\/.*\n { }
[ \f\t\n\r] { }
. 	{ 
  std::cerr << "Unknown character: " << yytext[0] 
    << " (" << (int)yytext[0] << ")" << std::endl; 
  yyextra->Error("Unknown character"); 
  return ERROR; 
}

%%
