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
#ifndef cmListFileLexer_h
#define cmListFileLexer_h

typedef enum cmListFileLexer_Type_e
{
  cmListFileLexer_Token_None,
  cmListFileLexer_Token_Newline,
  cmListFileLexer_Token_Identifier,
  cmListFileLexer_Token_ParenLeft,
  cmListFileLexer_Token_ParenRight,
  cmListFileLexer_Token_ArgumentUnquoted,
  cmListFileLexer_Token_ArgumentQuoted,
  cmListFileLexer_Token_Error
} cmListFileLexer_Type;

typedef struct cmListFileLexer_Token_s cmListFileLexer_Token;
struct cmListFileLexer_Token_s
{
  cmListFileLexer_Type type;
  char* text;
  int length;
  int line;
  int column;
};

typedef struct cmListFileLexer_s cmListFileLexer;

#ifdef __cplusplus
extern "C"
{
#endif

cmListFileLexer* cmListFileLexer_New();
int cmListFileLexer_SetFileName(cmListFileLexer*, const char*);
cmListFileLexer_Token* cmListFileLexer_Scan(cmListFileLexer*);
long cmListFileLexer_GetCurrentLine(cmListFileLexer*);
long cmListFileLexer_GetCurrentColumn(cmListFileLexer*);
void cmListFileLexer_Delete(cmListFileLexer*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
