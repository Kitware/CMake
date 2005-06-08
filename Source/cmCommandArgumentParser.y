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

Run bison like this:

  bison --yacc --name-prefix=cmCommandArgument_yy --defines=cmCommandArgumentParserTokens.h -ocmCommandArgumentParser.cxx cmCommandArgumentParser.y

Modify cmCommandArgumentParser.cxx:
  - remove TABs
  - add __HP_aCC to the #if test for yyerrorlab warning suppression

*/

/* Configure the parser to use a lexer object.  */
#define YYPARSE_PARAM yyscanner
#define YYLEX_PARAM yyscanner
#define YYERROR_VERBOSE 1
#define cmCommandArgument_yyerror(x) \
        cmCommandArgumentError(yyscanner, x)
#define yyGetParser (cmCommandArgument_yyget_extra(yyscanner))

/*-------------------------------------------------------------------------*/
#include "cmCommandArgumentParserHelper.h" /* Interface to parser object.  */
#include "cmCommandArgumentLexer.h"  /* Interface to lexer object.  */
#include "cmCommandArgumentParserTokens.h" /* Need YYSTYPE for YY_DECL.  */

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Internal utility functions.  */
static void cmCommandArgumentError(yyscan_t yyscanner, const char* message);

#define YYDEBUG 1
#define YYMAXDEPTH 10000000


#define calCheckEmpty(cnt) yyGetParser->CheckEmpty(__LINE__, cnt, yyvsp);
#define calElementStart(cnt) yyGetParser->PrepareElement(&yyval)
/* Disable some warnings in the generated code.  */
#ifdef __BORLANDC__
# pragma warn -8004 /* Variable assigned a value that is not used.  */
#endif
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
# pragma warning (disable: 4065) /* Switch statement contains default but no case. */
#endif
%}

/* Generate a reentrant parser object.  */
%pure_parser

/*
%union {
  char* string;
}
*/

/*-------------------------------------------------------------------------*/
/* Tokens */
%token cal_NCURLY
%token cal_DCURLY
%token cal_DOLLAR
%token cal_LCURLY
%token cal_RCURLY
%token cal_NAME
%token cal_SYMBOL
%token cal_AT
%token cal_ERROR
%token cal_ATNAME

/*-------------------------------------------------------------------------*/
/* grammar */
%%


Start:
Goal
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = 0;
  yyGetParser->SetResult($<str>1);
}

Goal:
{
  calElementStart(0);
  calCheckEmpty(0);
  $<str>$ = 0;
}
|
String Goal
{
  calElementStart(2);
  calCheckEmpty(2);
  $<str>$ = yyGetParser->CombineUnions($<str>1, $<str>2);
}

String:
TextWithRCurly
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}
|
Variable
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}

MultipleIds:
{
  calElementStart(0);
  calCheckEmpty(0);
}
|
ID MultipleIds
{
  calElementStart(2);
  calCheckEmpty(2);
  $<str>$ = yyGetParser->CombineUnions($<str>1, $<str>2);
}

ID:
Text
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}
|
Variable
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}

Text:
cal_NAME
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}
|
cal_SYMBOL
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}

TextWithRCurly:
Text
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}
|
cal_AT
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}
|
cal_DOLLAR
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}
|
cal_LCURLY
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}
|
cal_RCURLY
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = $<str>1;
}

Variable:
cal_NCURLY MultipleIds cal_RCURLY
{
  calElementStart(3);
  calCheckEmpty(3);
  $<str>$ = yyGetParser->ExpandSpecialVariable($<str>1,$<str>2);
  //std::cerr << __LINE__ << " here: [" << $<str>1 << "] [" << $<str>2 << "] [" << $<str>3 << "]" << std::endl;
}
|
cal_DCURLY MultipleIds cal_RCURLY
{
  calElementStart(3);
  calCheckEmpty(3);
  $<str>$ = yyGetParser->ExpandVariable($<str>2);
  //std::cerr << __LINE__ << " here: [" << $<str>1 << "] [" << $<str>2 << "] [" << $<str>3 << "]" << std::endl;
}
|
cal_ATNAME
{
  calElementStart(1);
  calCheckEmpty(1);
  $<str>$ = yyGetParser->ExpandVariable($<str>1);
}

%%
/* End of grammar */

/*--------------------------------------------------------------------------*/
void cmCommandArgumentError(yyscan_t yyscanner, const char* message)
{
  yyGetParser->Error(message);
}

