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
#ifndef cmDependsFortranParser_h
#define cmDependsFortranParser_h

/* Forward declare parser object type.  */
typedef struct cmDependsFortranParser_s cmDependsFortranParser;

#ifdef __cplusplus
extern "C"
{
#endif

/* Functions to enter/exit #include'd files in order.  */
int cmDependsFortranParser_FilePush(cmDependsFortranParser* parser,
                                    const char* fname);
int cmDependsFortranParser_FilePop(cmDependsFortranParser* parser);

/* Callbacks for lexer.  */
int cmDependsFortranParser_Input(cmDependsFortranParser* parser,
                                 char* buffer, size_t bufferSize);
void cmDependsFortranParser_StringStart(cmDependsFortranParser* parser);
const char* cmDependsFortranParser_StringEnd(cmDependsFortranParser* parser);
void cmDependsFortranParser_StringAppend(cmDependsFortranParser* parser,
                                         char c);
void cmDependsFortranParser_SetInInterface(cmDependsFortranParser* parser,
                                           int in);
int cmDependsFortranParser_GetInInterface(cmDependsFortranParser* parser);

/* Callbacks for parser.  */
void cmDependsFortranParser_Error(cmDependsFortranParser* parser,
                                  const char* message);
void cmDependsFortranParser_RuleUse(cmDependsFortranParser* parser,
                                    const char* name);
void cmDependsFortranParser_RuleInclude(cmDependsFortranParser* parser,
                                        const char* name);
void cmDependsFortranParser_RuleModule(cmDependsFortranParser* parser,
                                       const char* name);
void cmDependsFortranParser_RuleDefine(cmDependsFortranParser* parser,
                                       const char* name);
void cmDependsFortranParser_RuleUndef(cmDependsFortranParser* parser,
                                      const char* name);
void cmDependsFortranParser_RuleIfdef(cmDependsFortranParser* parser,
                                      const char* name);
void cmDependsFortranParser_RuleIfndef(cmDependsFortranParser* parser,
                                       const char* name);
void cmDependsFortranParser_RuleIf(cmDependsFortranParser* parser);
void cmDependsFortranParser_RuleElif(cmDependsFortranParser* parser);
void cmDependsFortranParser_RuleElse(cmDependsFortranParser* parser);
void cmDependsFortranParser_RuleEndif(cmDependsFortranParser* parser);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
