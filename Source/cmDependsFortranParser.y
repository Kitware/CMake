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

Run bison like this:

  bison --yacc --name-prefix=cmDependsFortran_yy --defines=cmDependsFortranParserTokens.h -ocmDependsFortranParser.cxx cmDependsFortranParser.y

Modify cmDependsFortranParser.cxx:
  - remove TABs
  - add __HP_aCC to the #if test for yyerrorlab warning suppression

*/

/*-------------------------------------------------------------------------*/
#define cmDependsFortranParser_cxx
#include "cmDependsFortranParser.h" /* Interface to parser object.  */
#include "cmDependsFortranParserTokens.h" /* Need YYSTYPE for YY_DECL.  */

/* Configure the parser to use a lexer object.  */
#define YYPARSE_PARAM yyscanner
#define YYLEX_PARAM yyscanner
#define YYERROR_VERBOSE 1
#define cmDependsFortran_yyerror(x) \
        cmDependsFortranError(yyscanner, x)

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Helper function to forward error callback.  */
static void cmDependsFortranError(yyscan_t yyscanner, const char* message)
{
  cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
  cmDependsFortranParser_Error(parser, message);
}

/* Disable some warnings in the generated code.  */
#ifdef __BORLANDC__
# pragma warn -8004 /* Variable assigned a value that is not used.  */
#endif
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
# pragma warning (disable: 4065) /* Switch contains default but no case. */
#endif
%}

/* Generate a reentrant parser object.  */
%pure_parser

%union {
  char* string;
}

/*-------------------------------------------------------------------------*/
/* Tokens */
%token USE F_INCLUDE MODULE EOSTMT
%token CPP_INCLUDE F90PPR_INCLUDE COCO_INCLUDE
%token F90PPR_DEFINE CPP_DEFINE F90PPR_UNDEF CPP_UNDEF
%token CPP_IFDEF CPP_IFNDEF CPP_IF CPP_ELSE CPP_ELIF CPP_ENDIF
%token F90PPR_IFDEF F90PPR_IFNDEF F90PPR_IF F90PPR_ELSE F90PPR_ELIF F90PPR_ENDIF
%token UNTERMINATED_STRING
%token <string> CPP_TOENDL STRING WORD

/*-------------------------------------------------------------------------*/
/* grammar */
%%

code: /* empty */ | code stmt ;

stmt:
  USE WORD other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleUse(parser, $2);
    free($2);
    }
| include STRING other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleInclude(parser, $2);
    free($2);
    }
| CPP_INCLUDE WORD other eostmt /* Ignore */
| MODULE WORD eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleModule(parser, $2);
    free($2);
    }
| define WORD other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleDefine(parser, $2);
    free($2);
    }
| undef WORD other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleUndef(parser, $2);
    free($2);
    }
| ifdef WORD other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfdef(parser, $2);
    free($2);
    }
| ifndef WORD other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfndef(parser, $2);
    free($2);
    }
| if other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIf(parser);
    }
| elif other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElif(parser);
    }
| else other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElse(parser);
    }
| endif other eostmt
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleEndif(parser);
    }
| other eostmt /* Ignore */
;

eostmt: /* empty */ | EOSTMT ;
include: F_INCLUDE | CPP_INCLUDE | F90PPR_INCLUDE | COCO_INCLUDE ;
define: CPP_DEFINE | F90PPR_DEFINE ;
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
| UNTERMINATED_STRING
;

%%
/* End of grammar */
