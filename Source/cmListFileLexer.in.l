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
/*

This file must be translated to C and modified to build everywhere.

Run flex like this:

  flex --prefix=cmListFileLexer_yy -ocmListFileLexer.c cmListFileLexer.in.l

Modify cmListFileLexer.c:
  - remove TABs
  - remove use of the 'register' storage class specifier
  - remove the yyunput function
  - add a statement "(void)yyscanner;" to the top of these methods:
      yy_fatal_error, cmListFileLexer_yyalloc, cmListFileLexer_yyrealloc, cmListFileLexer_yyfree
  - remove statement "yyscanner = NULL;" from cmListFileLexer_yylex_destroy
  - remove all YY_BREAK lines occurring right after return statements
  - remove the isatty forward declaration

*/

#include "cmStandardLexer.h"

/* Setup the proper cmListFileLexer_yylex declaration.  */
#define YY_EXTRA_TYPE cmListFileLexer*
#define YY_DECL int cmListFileLexer_yylex (yyscan_t yyscanner, cmListFileLexer* lexer)

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
  { result = cmListFileLexerInput(cmListFileLexer_yyget_extra(yyscanner), buf, max_size); }

/*--------------------------------------------------------------------------*/
%}

%option reentrant
%option yylineno
%option noyywrap
%pointer
%x STRING

MAKEVAR \$\([A-Za-z0-9_]*\)
UNQUOTED ([^ \t\r\n\(\)#\\\"]|\\.)
LEGACY {MAKEVAR}|{UNQUOTED}|\"({MAKEVAR}|{UNQUOTED}|[ \t])*\"

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

[A-Za-z_][A-Za-z0-9_]* {
  lexer->token.type = cmListFileLexer_Token_Identifier;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

({UNQUOTED})({UNQUOTED})* {
  lexer->token.type = cmListFileLexer_Token_ArgumentUnquoted;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

({MAKEVAR}|{UNQUOTED})({LEGACY})* {
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

<STRING>([^\\\n\"]|\\.)+ {
  cmListFileLexerAppend(lexer, yytext, yyleng);
  lexer->column += yyleng;
}

<STRING>\\\n {
  cmListFileLexerAppend(lexer, yytext, yyleng);
  ++lexer->line;
  lexer->column = 1;
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

<STRING><<EOF>> {
  lexer->token.type = cmListFileLexer_Token_BadString;
  BEGIN(INITIAL);
  return 1;
}

[ \t\r]+ {
  lexer->token.type = cmListFileLexer_Token_Space;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

. {
  lexer->token.type = cmListFileLexer_Token_BadCharacter;
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
    cmListFileLexer_yylex_init(&lexer->scanner);
    cmListFileLexer_yyset_extra(lexer, lexer->scanner);
    }
}

/*--------------------------------------------------------------------------*/
static void cmListFileLexerDestroy(cmListFileLexer* lexer)
{
  cmListFileLexerSetToken(lexer, 0, 0);
  if(lexer->file || lexer->string_buffer)
    {
    cmListFileLexer_yylex_destroy(lexer->scanner);
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
  if(cmListFileLexer_yylex(lexer->scanner, lexer))
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

/*--------------------------------------------------------------------------*/
const char* cmListFileLexer_GetTypeAsString(cmListFileLexer* lexer,
                                            cmListFileLexer_Type type)
{
  (void)lexer;
  switch(type)
    {
    case cmListFileLexer_Token_None: return "nothing";
    case cmListFileLexer_Token_Space: return "space";
    case cmListFileLexer_Token_Newline: return "newline";
    case cmListFileLexer_Token_Identifier: return "identifier";
    case cmListFileLexer_Token_ParenLeft: return "left paren";
    case cmListFileLexer_Token_ParenRight: return "right paren";
    case cmListFileLexer_Token_ArgumentUnquoted: return "unquoted argument";
    case cmListFileLexer_Token_ArgumentQuoted: return "quoted argument";
    case cmListFileLexer_Token_BadCharacter: return "bad character";
    case cmListFileLexer_Token_BadString: return "unterminated string";
    }
  return "unknown token";
}
