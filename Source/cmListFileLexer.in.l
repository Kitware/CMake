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

  flex -ocmListFileLexer.c cmListFileLexer.in.l

Modify cmListFileLexer.c:
  - remove TABs
  - remove the yyunput function
  - add a statement "(void)yyscanner;" to the top of these methods:
      yy_fatal_error, yyalloc, yyrealloc, yyfree
  - remove all YY_BREAK lines occurring right after return statements

*/

/* Disable features we do not need. */
#define YY_NEVER_INTERACTIVE 1
#define YY_NO_UNPUT 1
#define YY_NO_UNISTD_H 1
#define ECHO

/* Setup the proper yylex declaration.  */
#define YY_EXTRA_TYPE cmListFileLexer*
#define YY_DECL int yylex (yyscan_t yyscanner, cmListFileLexer* lexer)

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

#include "cmListFileLexer.h"

/*--------------------------------------------------------------------------*/
struct cmListFileLexer_s
{
  cmListFileLexer_Token token;
  int line;
  int column;
  int size;
  FILE* file;
  char* string_buffer;
  char* string_position;
  int string_left;
  yyscan_t scanner;
};

static void cmListFileLexerSetToken(cmListFileLexer* lexer, const char* text,
                                    int length);
static void cmListFileLexerAppend(cmListFileLexer* lexer, const char* text,
                                  int length);
static int cmListFileLexerInput(cmListFileLexer* lexer, char* buffer,
                                size_t bufferSize);
static void cmListFileLexerInit(cmListFileLexer* lexer);
static void cmListFileLexerDestroy(cmListFileLexer* lexer);

/* Replace the lexer input function.  */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
  { result = cmListFileLexerInput(yyget_extra(yyscanner), buf, max_size); }

/*--------------------------------------------------------------------------*/
%}

%option reentrant
%option yylineno
%option noyywrap
%pointer
%x STRING

%%

\n {
  lexer->token.type = cmListFileLexer_Token_Newline;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  ++lexer->line;
  lexer->column = 1;
  return 1;
}

#.* {
  lexer->column += yyleng;
}

\( {
  lexer->token.type = cmListFileLexer_Token_ParenLeft;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

\) {
  lexer->token.type = cmListFileLexer_Token_ParenRight;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

[A-Za-z_][A-Za-z0-9_]+ {
  lexer->token.type = cmListFileLexer_Token_Identifier;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

([^ \t\r\n\(\)#\\\"]|\\.)([^ \t\r\n\(\)#\\]|\\.)* {
  lexer->token.type = cmListFileLexer_Token_ArgumentUnquoted;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

\" {
  lexer->token.type = cmListFileLexer_Token_ArgumentQuoted;
  cmListFileLexerSetToken(lexer, "", 0);
  lexer->column += yyleng;
  BEGIN(STRING);
}

<STRING>([^\\\n\"]|\\(.|\n))+ {
  cmListFileLexerAppend(lexer, yytext, yyleng);
  lexer->column += yyleng;
}

<STRING>\n {
  cmListFileLexerAppend(lexer, yytext, yyleng);
  ++lexer->line;
  lexer->column = 1;
}

<STRING>\" {
  lexer->column += yyleng;
  BEGIN(INITIAL);
  return 1;
}

<STRING>. {
  cmListFileLexerAppend(lexer, yytext, yyleng);
  lexer->column += yyleng;
}

[ \t\r] {
  lexer->column += yyleng;
}

. {
  lexer->token.type = cmListFileLexer_Token_Error;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

<<EOF>> {
  lexer->token.type = cmListFileLexer_Token_None;
  cmListFileLexerSetToken(lexer, 0, 0);
  return 0;
}

%%

/*--------------------------------------------------------------------------*/
static void cmListFileLexerSetToken(cmListFileLexer* lexer, const char* text,
                                    int length)
{
  /* Set the token line and column number.  */
  lexer->token.line = lexer->line;
  lexer->token.column = lexer->column;

  /* Use the same buffer if possible.  */
  if(lexer->token.text)
    {
    if(text && length < lexer->size)
      {
      strcpy(lexer->token.text, text);
      lexer->token.length = length;
      return;
      }
    free(lexer->token.text);
    lexer->token.text = 0;
    lexer->size = 0;
    }

  /* Need to extend the buffer.  */
  if(text)
    {
    lexer->token.text = strdup(text);
    lexer->token.length = length;
    lexer->size = length+1;
    }
  else
    {
    lexer->token.length = 0;
    }
}

/*--------------------------------------------------------------------------*/
static void cmListFileLexerAppend(cmListFileLexer* lexer, const char* text,
                                  int length)
{
  char* temp;
  int newSize;

  /* If the appended text will fit in the buffer, do not reallocate.  */
  newSize = lexer->token.length + length + 1;
  if(lexer->token.text && newSize <= lexer->size)
    {
    strcpy(lexer->token.text+lexer->token.length, text);
    lexer->token.length += length;
    return;
    }

  /* We need to extend the buffer.  */
  temp = malloc(newSize);
  if(lexer->token.text)
    {
    memcpy(temp, lexer->token.text, lexer->token.length);
    free(lexer->token.text);
    }
  memcpy(temp+lexer->token.length, text, length);
  temp[lexer->token.length+length] = 0;
  lexer->token.text = temp;
  lexer->token.length += length;
  lexer->size = newSize;
}

/*--------------------------------------------------------------------------*/
static int cmListFileLexerInput(cmListFileLexer* lexer, char* buffer,
                                size_t bufferSize)
{
  if(lexer)
    {
    if(lexer->file)
      {
      return (int)fread(buffer, 1, bufferSize, lexer->file);
      }
    else if(lexer->string_left)
      {
      int length = lexer->string_left;
      if((int)bufferSize < length) { length = (int)bufferSize; }
      memcpy(buffer, lexer->string_position, length);
      lexer->string_position += length;
      lexer->string_left -= length;
      return length;
      }
    }
  return 0;
}

/*--------------------------------------------------------------------------*/
static void cmListFileLexerInit(cmListFileLexer* lexer)
{
  if(lexer->file || lexer->string_buffer)
    {
    yylex_init(&lexer->scanner);
    yyset_extra(lexer, lexer->scanner);
    }
}

/*--------------------------------------------------------------------------*/
static void cmListFileLexerDestroy(cmListFileLexer* lexer)
{
  if(lexer->file || lexer->string_buffer)
    {
    yylex_destroy(lexer->scanner);
    if(lexer->file)
      {
      fclose(lexer->file);
      lexer->file = 0;
      }
    if(lexer->string_buffer)
      {
      free(lexer->string_buffer);
      lexer->string_buffer = 0;
      lexer->string_left = 0;
      lexer->string_position = 0;
      }
    }
}

/*--------------------------------------------------------------------------*/
cmListFileLexer* cmListFileLexer_New()
{
  cmListFileLexer* lexer = (cmListFileLexer*)malloc(sizeof(cmListFileLexer));
  if(!lexer)
    {
    return 0;
    }
  memset(lexer, 0, sizeof(*lexer));
  lexer->line = 1;
  lexer->column = 1;
  return lexer;
}

/*--------------------------------------------------------------------------*/
void cmListFileLexer_Delete(cmListFileLexer* lexer)
{
  cmListFileLexer_SetFileName(lexer, 0);
  free(lexer);
}

/*--------------------------------------------------------------------------*/
int cmListFileLexer_SetFileName(cmListFileLexer* lexer, const char* name)
{
  int result = 1;
  cmListFileLexerDestroy(lexer);
  if(name)
    {
    lexer->file = fopen(name, "r");
    if(!lexer->file)
      {
      result = 0;
      }
    }
  cmListFileLexerInit(lexer);
  return result;
}

/*--------------------------------------------------------------------------*/
int cmListFileLexer_SetString(cmListFileLexer* lexer, const char* text)
{
  int result = 1;
  cmListFileLexerDestroy(lexer);
  if(text)
    {
    int length = (int)strlen(text);
    lexer->string_buffer = (char*)malloc(length+1);
    if(lexer->string_buffer)
      {
      strcpy(lexer->string_buffer, text);
      lexer->string_position = lexer->string_buffer;
      lexer->string_left = length;
      }
    else
      {
      result = 0;
      }
    }
  cmListFileLexerInit(lexer);
  return result;
}

/*--------------------------------------------------------------------------*/
cmListFileLexer_Token* cmListFileLexer_Scan(cmListFileLexer* lexer)
{
  if(!lexer->file)
    {
    return 0;
    }
  if(yylex(lexer->scanner, lexer))
    {
    return &lexer->token;
    }
  else
    {
    cmListFileLexer_SetFileName(lexer, 0);
    return 0;
    }
}

/*--------------------------------------------------------------------------*/
long cmListFileLexer_GetCurrentLine(cmListFileLexer* lexer)
{
  if(lexer->file)
    {
    return lexer->line;
    }
  else
    {
    return 0;
    }
}

/*--------------------------------------------------------------------------*/
long cmListFileLexer_GetCurrentColumn(cmListFileLexer* lexer)
{
  if(lexer->file)
    {
    return lexer->column;
    }
  else
    {
    return 0;
    }
}
