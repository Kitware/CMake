%{
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
/*

This file must be translated to C and modified to build everywhere.

Run flex >= 2.6 like this:

  flex --nounistd -DFLEXINT_H --noline -ocmListFileLexer.c cmListFileLexer.in.l

Modify cmListFileLexer.c:
  - remove trailing whitespace:              sed -i 's/\s*$//' cmListFileLexer.c
  - remove blank lines at end of file:       sed -i '${/^$/d;}' cmListFileLexer.c
  - #include "cmStandardLexer.h" at the top: sed -i '1i#include "cmStandardLexer.h"' cmListFileLexer.c

*/

/* IWYU pragma: no_forward_declare yyguts_t */

#ifdef _WIN32
#include "cmsys/Encoding.h"
#endif

/* Setup the proper cmListFileLexer_yylex declaration.  */
#define YY_EXTRA_TYPE cmListFileLexer*
#define YY_DECL int cmListFileLexer_yylex (yyscan_t yyscanner, cmListFileLexer* lexer)

#include "cmListFileLexer.h"

/*--------------------------------------------------------------------------*/
struct cmListFileLexer_s
{
  cmListFileLexer_Token token;
  int bracket;
  int comment;
  int line;
  int column;
  size_t size;
  FILE* file;
  size_t cr;
  char read_buffer[4];
  size_t read_size;
  size_t read_position;
  char* string_buffer;
  char* string_position;
  size_t string_left;
  yyscan_t scanner;
};

static void cmListFileLexerSetToken(cmListFileLexer* lexer, const char* text,
                                    size_t length);
static void cmListFileLexerAppend(cmListFileLexer* lexer, const char* text,
                                  size_t length);
static int cmListFileLexerInput(cmListFileLexer* lexer, char* buffer,
                                size_t bufferSize);
static void cmListFileLexerInit(cmListFileLexer* lexer);
static void cmListFileLexerDestroy(cmListFileLexer* lexer);

/* Replace the lexer input function.  */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
  do { result = cmListFileLexerInput(cmListFileLexer_yyget_extra(yyscanner), buf, max_size); } while (0)

/*--------------------------------------------------------------------------*/
%}

%option prefix="cmListFileLexer_yy"

%option reentrant
%option yylineno
%option noyywrap
%pointer
%x STRING
%x BRACKET
%x BRACKETEND
%x COMMENT

MAKEVAR \$\([A-Za-z0-9_]*\)
UNQUOTED ([^ \t\r\n\(\)#\\\"[=]|\\[^\n])
LEGACY {MAKEVAR}|{UNQUOTED}|\"({MAKEVAR}|{UNQUOTED}|[ \t[=])*\"

%%

<INITIAL,COMMENT>\n {
  lexer->token.type = cmListFileLexer_Token_Newline;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  ++lexer->line;
  lexer->column = 1;
  BEGIN(INITIAL);
  return 1;
}

#?\[=*\[\n? {
  const char* bracket = yytext;
  size_t length = yyleng;
  lexer->comment = yytext[0] == '#';
  if (lexer->comment) {
    lexer->token.type = cmListFileLexer_Token_CommentBracket;
    bracket += 1;
    --length;
  } else {
    lexer->token.type = cmListFileLexer_Token_ArgumentBracket;
  }
  cmListFileLexerSetToken(lexer, "", 0);
  lexer->bracket = (char*)memchr(bracket + 1, '[', length - 1) - bracket;
  if (yytext[yyleng-1] == '\n') {
    ++lexer->line;
    lexer->column = 1;
  } else {
    lexer->column += yyleng;
  }
  BEGIN(BRACKET);
}

# {
  lexer->column += yyleng;
  BEGIN(COMMENT);
}

<COMMENT>[^\n]* {
  if (memchr(yytext, '\0', yyleng) != NULL) {
    lexer->token.type = cmListFileLexer_Token_BadCharacter;
  }
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

<BRACKET>\]=* {
  /* Handle ]]====]=======]*/
  cmListFileLexerAppend(lexer, yytext, yyleng);
  lexer->column += yyleng;
  if (yyleng == lexer->bracket) {
    BEGIN(BRACKETEND);
  }
}

<BRACKETEND>\] {
  lexer->column += yyleng;
  /* Erase the partial bracket from the token.  */
  lexer->token.length -= lexer->bracket;
  BEGIN(INITIAL);
  return 1;
}

<BRACKET>([^]\n])+ {
  if (memchr(yytext, '\0', yyleng) != NULL) {
    lexer->token.type = cmListFileLexer_Token_BadCharacter;
  }
  cmListFileLexerAppend(lexer, yytext, yyleng);
  lexer->column += yyleng;
}

<BRACKET,BRACKETEND>\n {
  cmListFileLexerAppend(lexer, yytext, yyleng);
  ++lexer->line;
  lexer->column = 1;
  BEGIN(BRACKET);
}

<BRACKET,BRACKETEND>[^\n] {
  if (memchr(yytext, '\0', yyleng) != NULL) {
    lexer->token.type = cmListFileLexer_Token_BadCharacter;
  }
  cmListFileLexerAppend(lexer, yytext, yyleng);
  lexer->column += yyleng;
  BEGIN(BRACKET);
}

<BRACKET,BRACKETEND><<EOF>> {
  lexer->token.type = cmListFileLexer_Token_BadBracket;
  BEGIN(INITIAL);
  return 1;
}

({UNQUOTED}|=|\[=*{UNQUOTED})({UNQUOTED}|[[=])* {
  if (memchr(yytext, '\0', yyleng) != NULL) {
    /* An unquoted argument that contains a null character. */
    lexer->token.type = cmListFileLexer_Token_BadCharacter;
  } else {
    lexer->token.type = cmListFileLexer_Token_ArgumentUnquoted;
  }
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

({MAKEVAR}|{UNQUOTED}|=|\[=*{LEGACY})({LEGACY}|[[=])* {
  lexer->token.type = cmListFileLexer_Token_ArgumentUnquoted;
  cmListFileLexerSetToken(lexer, yytext, yyleng);
  lexer->column += yyleng;
  return 1;
}

\[ {
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

<STRING>([^\\\n\"]|\\[^\n])+ {
  if (memchr(yytext, '\0', yyleng) != NULL) {
    lexer->token.type = cmListFileLexer_Token_BadCharacter;
  }
  cmListFileLexerAppend(lexer, yytext, yyleng);
  lexer->column += yyleng;
}

<STRING>\\\n {
  /* Continuation: text is not part of string */
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

<STRING>[^\n] {
  if (memchr(yytext, '\0', yyleng) != NULL) {
    lexer->token.type = cmListFileLexer_Token_BadCharacter;
  }
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
                                    size_t length)
{
  /* Set the token line and column number.  */
  lexer->token.line = lexer->line;
  lexer->token.column = lexer->column;

  /* Use the same buffer if possible.  */
  if (lexer->token.text) {
    if (text && length < lexer->size) {
      memcpy(lexer->token.text, text, length);
      lexer->token.length = length;
      return;
    }
    free(lexer->token.text);
    lexer->token.text = 0;
    lexer->size = 0;
  }

  /* Need to extend the buffer.  */
  if (length > 0) {
    lexer->token.text = (char*)malloc(length);
    memcpy(lexer->token.text, text, length);
    lexer->token.length = length;
    lexer->size = length;
  } else {
    lexer->token.length = 0;
  }
}

/*--------------------------------------------------------------------------*/
static void cmListFileLexerAppend(cmListFileLexer* lexer, const char* text,
                                  size_t length)
{
  char* temp;
  size_t newSize;

  /* If the appended text will fit in the buffer, do not reallocate.  */
  newSize = lexer->token.length + length;
  if (lexer->token.text && newSize <= lexer->size) {
    memcpy(lexer->token.text + lexer->token.length, text, length);
    lexer->token.length += length;
    return;
  }

  /* We need to extend the buffer.  */
  temp = malloc(newSize);
  if (lexer->token.text) {
    memcpy(temp, lexer->token.text, lexer->token.length);
    free(lexer->token.text);
  }
  memcpy(temp + lexer->token.length, text, length);
  lexer->token.text = temp;
  lexer->token.length += length;
  lexer->size = newSize;
}

/*--------------------------------------------------------------------------*/
static int cmListFileLexerInput(cmListFileLexer* lexer, char* buffer,
                                size_t bufferSize)
{
  if (lexer) {
    if (lexer->file) {
      /* Convert CRLF -> LF explicitly.  The C FILE "t"ext mode
         does not convert newlines on all platforms.  Move any
         trailing CR to the start of the buffer for the next read. */
      size_t cr = lexer->cr;
      size_t n = 0;
      buffer[0] = '\r';

      size_t actualBufferSize = bufferSize - cr;
      char* p = buffer + cr;
      size_t readLeft = lexer->read_size - lexer->read_position;

      /* Absorb the bytes that were read during BOM detection, if any. */
      if (readLeft > 0) {
        size_t actualReadSize =
          actualBufferSize >= readLeft ? readLeft : actualBufferSize;
        memcpy(p, lexer->read_buffer + lexer->read_position, actualReadSize);
        lexer->read_position += actualReadSize;
        p += actualReadSize;
        n += actualReadSize;
        actualBufferSize -= actualReadSize;
      }

      n += fread(p, 1, actualBufferSize, lexer->file);

      if (n) {
        char* o = buffer;
        const char* i = buffer;
        const char* e;
        n += cr;
        cr = (buffer[n - 1] == '\r') ? 1 : 0;
        e = buffer + n - cr;
        while (i != e) {
          if (i[0] == '\r' && i[1] == '\n') {
            ++i;
          }
          *o++ = *i++;
        }
        n = o - buffer;
      } else {
        n = cr;
        cr = 0;
      }
      lexer->cr = cr;
      return n;
    } else if (lexer->string_left) {
      size_t length = lexer->string_left;
      if (bufferSize < length) {
        length = bufferSize;
      }
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
  if (lexer->file || lexer->string_buffer) {
    cmListFileLexer_yylex_init(&lexer->scanner);
    cmListFileLexer_yyset_extra(lexer, lexer->scanner);
  }
}

/*--------------------------------------------------------------------------*/
static void cmListFileLexerDestroy(cmListFileLexer* lexer)
{
  cmListFileLexerSetToken(lexer, 0, 0);
  if (lexer->file || lexer->string_buffer) {
    cmListFileLexer_yylex_destroy(lexer->scanner);
    if (lexer->file) {
      fclose(lexer->file);
      lexer->file = 0;
    }
    if (lexer->read_size != 0) {
      memset(lexer->read_buffer, 0, sizeof(lexer->read_buffer));
      lexer->read_size = 0;
      lexer->read_position = 0;
    }
    if (lexer->string_buffer) {
      free(lexer->string_buffer);
      lexer->string_buffer = 0;
      lexer->string_left = 0;
      lexer->string_position = 0;
    }
  }
}

/*--------------------------------------------------------------------------*/
cmListFileLexer* cmListFileLexer_New(void)
{
  cmListFileLexer* lexer = (cmListFileLexer*)malloc(sizeof(cmListFileLexer));
  if (!lexer) {
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
  cmListFileLexer_SetFileName(lexer, 0, 0);
  free(lexer);
}

/*--------------------------------------------------------------------------*/
static cmListFileLexer_BOM cmListFileLexer_ReadBOM(FILE* f,
                                                   unsigned char readBuffer[4],
                                                   size_t* readSize)
{
  /* Read the up to four bytes that might correspond to a BOM. In case these
     bytes turn out not to represent a BOM, save them for later consumption in
     order to avoid seeking the file (which might not be seekable, e.g., if
     it's a pipe). */
  unsigned char* b = readBuffer;

  size_t n = fread(b, 1, 2, f);
  *readSize = n; /* Initialize first and then accumulate */

  if (n == 2) {
    if (b[0] == 0xEF && b[1] == 0xBB) {
      n = fread(b + 2, 1, 1, f);
      *readSize += n;

      if (n == 1) {
        if (b[2] == 0xBF) {
          *readSize = 0; /* We consumed the BOM: discard it */
          return cmListFileLexer_BOM_UTF8;
        }
      }
    } else if (b[0] == 0xFE && b[1] == 0xFF) {
      *readSize = 0; /* We consumed the BOM: discard it */
      /* UTF-16 BE */
      return cmListFileLexer_BOM_UTF16BE;
    } else if (b[0] == 0 && b[1] == 0) {
      n = fread(b + 2, 1, 2, f);
      *readSize += n;

      if (n == 2) {
        if (b[2] == 0xFE && b[3] == 0xFF) {
          *readSize = 0; /* We consumed the BOM: discard it */
          return cmListFileLexer_BOM_UTF32BE;
        }
      }
    } else if (b[0] == 0xFF && b[1] == 0xFE) {
      n = fread(b + 2, 1, 2, f);
      *readSize += n;

      if (n == 2 && b[2] == 0 && b[3] == 0) {
        *readSize = 0; /* We consumed the BOM: discard it */
        return cmListFileLexer_BOM_UTF32LE;
      }

      /* In case we were able to subsequently read only a single byte out of two
         (i.e., three in total), the file must be corrupt and the BOM cannot
         represent a UTF-16-LE BOM since each code unit must consist of two
         bytes. This avoids incorrectly detecting an incomplete UTF-32-LE BOM as
         UTF-16-LE input. */
      if (n % 2 == 0) {
        *readSize = n; /* We consumed the read bytes as BOM only partially */
        memmove(b, b + 2, n);
        return cmListFileLexer_BOM_UTF16LE;
      }
    }
  }

  return cmListFileLexer_BOM_None;
}

/*--------------------------------------------------------------------------*/
int cmListFileLexer_SetFileName(cmListFileLexer* lexer, const char* name,
                                cmListFileLexer_BOM* bom)
{
  int result = 1;
  cmListFileLexerDestroy(lexer);
  if (name) {
#ifdef _WIN32
    wchar_t* wname = cmsysEncoding_DupToWide(name);
    lexer->file = _wfopen(wname, L"rb");
    free(wname);
#else
    lexer->file = fopen(name, "rb");
#endif
    if (lexer->file) {
      if (bom) {
        *bom = cmListFileLexer_ReadBOM(
          lexer->file, (unsigned char*)lexer->read_buffer, &lexer->read_size);
        lexer->read_position = 0;
      } else {
        memset(lexer->read_buffer, 0, sizeof(lexer->read_buffer));
        lexer->read_size = 0;
        lexer->read_position = 0;
      }
    } else {
      result = 0;
    }
  }
  cmListFileLexerInit(lexer);
  return result;
}

/*--------------------------------------------------------------------------*/
int cmListFileLexer_SetString(cmListFileLexer* lexer, char const* text,
                              size_t length)
{
  int result = 1;
  cmListFileLexerDestroy(lexer);
  /* text might be not NULL while length is 0. However, on some platforms
     malloc(0) will return NULL. To avoid signaling an error to the caller in
     such cases, ensure nonzero length. */
  size_t read_size = lexer->read_size - lexer->read_position;
  size_t string_size = read_size + length;
  if (string_size > 0) {
    lexer->string_buffer = (char*)malloc(string_size);
    if (lexer->string_buffer) {
      memcpy(lexer->string_buffer, lexer->read_buffer + lexer->read_position,
             read_size);
      memcpy(lexer->string_buffer + read_size, text, length);
      lexer->read_position += read_size;
      lexer->string_position = lexer->string_buffer;
      lexer->string_left = length;
    } else {
      result = 0;
    }
  }
  cmListFileLexerInit(lexer);
  return result;
}

/*--------------------------------------------------------------------------*/
cmListFileLexer_Token* cmListFileLexer_Scan(cmListFileLexer* lexer)
{
  if (!lexer->file && !lexer->string_buffer) {
    return 0;
  }
  if (cmListFileLexer_yylex(lexer->scanner, lexer)) {
    return &lexer->token;
  } else {
    cmListFileLexer_SetFileName(lexer, 0, 0);
    return 0;
  }
}

/*--------------------------------------------------------------------------*/
long cmListFileLexer_GetCurrentLine(cmListFileLexer* lexer)
{
  return lexer->line;
}

/*--------------------------------------------------------------------------*/
long cmListFileLexer_GetCurrentColumn(cmListFileLexer* lexer)
{
  return lexer->column;
}

/*--------------------------------------------------------------------------*/
const char* cmListFileLexer_GetTypeAsString(cmListFileLexer* lexer,
                                            cmListFileLexer_Type type)
{
  (void)lexer;
  switch (type) {
    case cmListFileLexer_Token_None:
      return "nothing";
    case cmListFileLexer_Token_Space:
      return "space";
    case cmListFileLexer_Token_Newline:
      return "newline";
    case cmListFileLexer_Token_Identifier:
      return "identifier";
    case cmListFileLexer_Token_ParenLeft:
      return "left paren";
    case cmListFileLexer_Token_ParenRight:
      return "right paren";
    case cmListFileLexer_Token_ArgumentUnquoted:
      return "unquoted argument";
    case cmListFileLexer_Token_ArgumentQuoted:
      return "quoted argument";
    case cmListFileLexer_Token_ArgumentBracket:
      return "bracket argument";
    case cmListFileLexer_Token_CommentBracket:
      return "bracket comment";
    case cmListFileLexer_Token_BadCharacter:
      return "bad character";
    case cmListFileLexer_Token_BadBracket:
      return "unterminated bracket";
    case cmListFileLexer_Token_BadString:
      return "unterminated string";
  }
  return "unknown token";
}
