%{
/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
/*-------------------------------------------------------------------------
  Portions of this source have been derived from makedepf90 version 2.8.8,

   Copyright (C) 2000--2006 Erik Edelmann <erik.edelmann@iki.fi>

  The code was originally distributed under the GPL but permission
  from the copyright holder has been obtained to distribute this
  derived work under the CMake license.
-------------------------------------------------------------------------*/

/*

This file must be translated to C and modified to build everywhere.

Run bison like this:

  bison --yacc --name-prefix=cmFortran_yy
        --defines=cmFortranParserTokens.h
         -ocmFortranParser.cxx
          cmFortranParser.y

Modify cmFortranParser.cxx:
  - "#if 0" out yyerrorlab block in range ["goto yyerrlab1", "yyerrlab1:"]
*/

/*-------------------------------------------------------------------------*/
#define cmFortranParser_cxx
#include "cmFortranParser.h" /* Interface to parser object.  */
#include "cmFortranParserTokens.h" /* Need YYSTYPE for YY_DECL.  */

#include <cmsys/String.h>

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Helper function to forward error callback from parser.  */
static void cmFortran_yyerror(yyscan_t yyscanner, const char* message)
{
  cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
  cmFortranParser_Error(parser, message);
}

static bool cmFortranParserIsKeyword(const char* word,
                                            const char* keyword)
{
  return cmsysString_strcasecmp(word, keyword) == 0;
}

/* Disable some warnings in the generated code.  */
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
# pragma warning (disable: 4065) /* Switch contains default but no case. */
# pragma warning (disable: 4701) /* Local variable may not be initialized.  */
# pragma warning (disable: 4702) /* Unreachable code.  */
# pragma warning (disable: 4127) /* Conditional expression is constant.  */
# pragma warning (disable: 4244) /* Conversion to smaller type, data loss. */
#endif
%}

/* Generate a reentrant parser object.  */
%define api.pure

/* Configure the parser to use a lexer object.  */
%lex-param   {yyscan_t yyscanner}
%parse-param {yyscan_t yyscanner}

%define parse.error verbose

%union {
  char* string;
}

/*-------------------------------------------------------------------------*/
/* Tokens */
%token EOSTMT ASSIGNMENT_OP GARBAGE
%token CPP_INCLUDE F90PPR_INCLUDE COCO_INCLUDE
%token F90PPR_DEFINE CPP_DEFINE F90PPR_UNDEF CPP_UNDEF
%token CPP_IFDEF CPP_IFNDEF CPP_IF CPP_ELSE CPP_ELIF CPP_ENDIF
%token F90PPR_IFDEF F90PPR_IFNDEF F90PPR_IF
%token F90PPR_ELSE F90PPR_ELIF F90PPR_ENDIF
%token COMMA DCOLON
%token <string> CPP_TOENDL
%token <number> UNTERMINATED_STRING
%token <string> STRING WORD
%token <string> CPP_INCLUDE_ANGLE

/*-------------------------------------------------------------------------*/
/* grammar */
%%

code: /* empty */ | code stmt;

stmt: keyword_stmt | assignment_stmt;

assignment_stmt: WORD ASSIGNMENT_OP other EOSTMT    /* Ignore */
    {
    free($1);
    }

keyword_stmt:
  WORD EOSTMT
    {
    if (cmFortranParserIsKeyword($1, "interface"))
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_SetInInterface(parser, true);
      }
    free($1);
    }
| WORD WORD other EOSTMT
    {
    if (cmFortranParserIsKeyword($1, "use"))
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_RuleUse(parser, $2);
      }
    else if (cmFortranParserIsKeyword($1, "module"))
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_RuleModule(parser, $2);
      }
    else if (cmFortranParserIsKeyword($1, "interface"))
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_SetInInterface(parser, true);
      }
    else if (cmFortranParserIsKeyword($2, "interface") &&
             cmFortranParserIsKeyword($1, "end"))
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_SetInInterface(parser, false);
      }
    free($1);
    free($2);
    }
| WORD DCOLON WORD other EOSTMT
    {
    if (cmFortranParserIsKeyword($1, "use"))
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_RuleUse(parser, $3);
      }
    free($1);
    free($3);
    }
| WORD COMMA WORD DCOLON WORD other EOSTMT
    {
    if (cmFortranParserIsKeyword($1, "use") &&
        cmFortranParserIsKeyword($3, "non_intrinsic") )
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_RuleUse(parser, $5);
      }
    free($1);
    free($3);
    free($5);
    }
| WORD STRING other EOSTMT /* Ignore */
    {
    if (cmFortranParserIsKeyword($1, "include"))
      {
      cmFortranParser* parser =
        cmFortran_yyget_extra(yyscanner);
      cmFortranParser_RuleInclude(parser, $2);
      }
    free($1);
    free($2);
    }
| CPP_INCLUDE_ANGLE other EOSTMT
    {
    cmFortranParser* parser =
      cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleInclude(parser, $1);
    free($1);
    }
| include STRING other EOSTMT
    {
    cmFortranParser* parser =
      cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleInclude(parser, $2);
    free($2);
    }
| define WORD other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleDefine(parser, $2);
    free($2);
    }
| undef WORD other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleUndef(parser, $2);
    free($2);
    }
| ifdef WORD other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleIfdef(parser, $2);
    free($2);
    }
| ifndef WORD other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleIfndef(parser, $2);
    free($2);
    }
| if other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleIf(parser);
    }
| elif other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleElif(parser);
    }
| else other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleElse(parser);
    }
| endif other EOSTMT
    {
    cmFortranParser* parser = cmFortran_yyget_extra(yyscanner);
    cmFortranParser_RuleEndif(parser);
    }
| WORD GARBAGE other EOSTMT             /* Ignore */
    {
    free($1);
    }
| GARBAGE other EOSTMT
| EOSTMT
| error
;



include: CPP_INCLUDE | F90PPR_INCLUDE | COCO_INCLUDE ;
define: CPP_DEFINE | F90PPR_DEFINE;
undef: CPP_UNDEF | F90PPR_UNDEF ;
ifdef: CPP_IFDEF | F90PPR_IFDEF ;
ifndef: CPP_IFNDEF | F90PPR_IFNDEF ;
if: CPP_IF | F90PPR_IF ;
elif: CPP_ELIF | F90PPR_ELIF ;
else: CPP_ELSE | F90PPR_ELSE ;
endif: CPP_ENDIF | F90PPR_ENDIF ;
other: /* empty */ | other misc_code ;

misc_code:
  WORD                { free ($1); }
| STRING              { free ($1); }
| GARBAGE
| ASSIGNMENT_OP
| DCOLON
| COMMA
| UNTERMINATED_STRING
;

%%
/* End of grammar */
