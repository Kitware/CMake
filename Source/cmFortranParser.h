/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmFortranParser_h
#define cmFortranParser_h

#include <stddef.h> /* size_t */

/* Forward declare parser object type.  */
typedef struct cmFortranParser_s cmFortranParser;

/* Functions to enter/exit #include'd files in order.  */
bool cmFortranParser_FilePush(cmFortranParser* parser,
                                    const char* fname);
bool cmFortranParser_FilePop(cmFortranParser* parser);

/* Callbacks for lexer.  */
int cmFortranParser_Input(cmFortranParser* parser,
                                 char* buffer, size_t bufferSize);


void cmFortranParser_StringStart(cmFortranParser* parser);
const char* cmFortranParser_StringEnd(cmFortranParser* parser);
void cmFortranParser_StringAppend(cmFortranParser* parser,
                                         char c);

void cmFortranParser_SetInInterface(cmFortranParser* parser,
                                           bool is_in);
bool cmFortranParser_GetInInterface(cmFortranParser* parser);


void cmFortranParser_SetInPPFalseBranch(cmFortranParser* parser,
                                               bool is_in);
bool cmFortranParser_GetInPPFalseBranch(cmFortranParser* parser);


void cmFortranParser_SetOldStartcond(cmFortranParser* parser,
                                            int arg);
int cmFortranParser_GetOldStartcond(cmFortranParser* parser);

/* Callbacks for parser.  */
void cmFortranParser_Error(cmFortranParser* parser,
                                  const char* message);
void cmFortranParser_RuleUse(cmFortranParser* parser,
                                    const char* name);
void cmFortranParser_RuleInclude(cmFortranParser* parser,
                                        const char* name);
void cmFortranParser_RuleModule(cmFortranParser* parser,
                                       const char* name);
void cmFortranParser_RuleDefine(cmFortranParser* parser,
                                       const char* name);
void cmFortranParser_RuleUndef(cmFortranParser* parser,
                                      const char* name);
void cmFortranParser_RuleIfdef(cmFortranParser* parser,
                                      const char* name);
void cmFortranParser_RuleIfndef(cmFortranParser* parser,
                                       const char* name);
void cmFortranParser_RuleIf(cmFortranParser* parser);
void cmFortranParser_RuleElif(cmFortranParser* parser);
void cmFortranParser_RuleElse(cmFortranParser* parser);
void cmFortranParser_RuleEndif(cmFortranParser* parser);

/* Define the parser stack element type.  */
typedef union cmFortran_yystype_u cmFortran_yystype;
union cmFortran_yystype_u
{
  char* string;
};

/* Setup the proper yylex interface.  */
#define YY_EXTRA_TYPE cmFortranParser*
#define YY_DECL \
int cmFortran_yylex(YYSTYPE* yylvalp, yyscan_t yyscanner)
#define YYSTYPE cmFortran_yystype
#define YYSTYPE_IS_DECLARED 1
#if !defined(cmFortranLexer_cxx)
# include "cmFortranLexer.h"
#endif
#if !defined(cmFortranLexer_cxx)
#if !defined(cmFortranParser_cxx)
# undef YY_EXTRA_TYPE
# undef YY_DECL
# undef YYSTYPE
# undef YYSTYPE_IS_DECLARED
#endif
#endif

#endif
