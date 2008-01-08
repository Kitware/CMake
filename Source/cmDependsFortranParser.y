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

  bison --yacc --name-prefix=cmDependsFortran_yy
        --defines=cmDependsFortranParserTokens.h
         -ocmDependsFortranParser.cxx
          cmDependsFortranParser.y

Modify cmDependsFortranParser.cxx:
  - remove TABs
  - Remove the yyerrorlab block in range ["goto yyerrlab1", "yyerrlab1:"]
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

static char charmap[] = {
    '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
    '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
    '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
    '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
    '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
    '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
    '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
    '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
    '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
    '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
    '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
    '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
    '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
    '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
    '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
    '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
    '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
    '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
    '\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
    '\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
    '\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
    '\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
    '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
    '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
    '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
    '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377'
};

inline int strcasecmpCM(const char *s1, const char *s2) 
{
  const char *cm = charmap;
  const char* us1 = s1;
  const char* us2 = s2;
  
  while(cm[*us1] == cm[*us2++])
    if(*us1++ == '\0') 
      {
      return(0);
      }
  return(cm[*us1] - cm[*--us2]);
}

static bool cmDependsFortranParserIsKeyword(const char* word,
                                            const char* keyword)
{
  return strcasecmpCM(word, keyword) == 0;
}

/* Disable some warnings in the generated code.  */
#ifdef __BORLANDC__
# pragma warn -8004 /* Variable assigned a value that is not used.  */
# pragma warn -8008 /* condition always returns true */
# pragma warn -8060 /* possibly incorrect assignment */
# pragma warn -8066 /* unreachable code */
#endif
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
%pure-parser

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
%token <string> CPP_TOENDL
%token <number> UNTERMINATED_STRING
%token <string> STRING WORD

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
    if (cmDependsFortranParserIsKeyword($1, "interface"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_SetInInterface(parser, true);
      }
    free($1);
    }
| WORD WORD other EOSTMT
    {
    if (cmDependsFortranParserIsKeyword($1, "use"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleUse(parser, $2);
      }
    else if (cmDependsFortranParserIsKeyword($1, "module"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleModule(parser, $2);
      }
    else if (cmDependsFortranParserIsKeyword($1, "interface"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_SetInInterface(parser, true);
      }
    else if (cmDependsFortranParserIsKeyword($2, "interface") &&
             cmDependsFortranParserIsKeyword($1, "end"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_SetInInterface(parser, false);
      }
    free($1);
    free($2);
    }
| WORD STRING other EOSTMT /* Ignore */
    {
    if (cmDependsFortranParserIsKeyword($1, "include"))
      {
      cmDependsFortranParser* parser =
        cmDependsFortran_yyget_extra(yyscanner);
      cmDependsFortranParser_RuleInclude(parser, $2);
      }
    free($1);
    free($2);
    }
| include STRING other EOSTMT
    {
    cmDependsFortranParser* parser =
      cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleInclude(parser, $2);
    free($2);
    }
| define WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleDefine(parser, $2);
    free($2);
    }
| undef WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleUndef(parser, $2);
    free($2);
    }
| ifdef WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfdef(parser, $2);
    free($2);
    }
| ifndef WORD other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfndef(parser, $2);
    free($2);
    }
| if other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIf(parser);
    }
| elif other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElif(parser);
    }
| else other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElse(parser);
    }
| endif other EOSTMT
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleEndif(parser);
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
| UNTERMINATED_STRING
;

%%
/* End of grammar */
