/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_CMFORTRAN_YY_CMFORTRANPARSERTOKENS_H_INCLUDED
# define YY_CMFORTRAN_YY_CMFORTRANPARSERTOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cmFortran_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    EOSTMT = 258,
    ASSIGNMENT_OP = 259,
    GARBAGE = 260,
    CPP_LINE_DIRECTIVE = 261,
    CPP_INCLUDE = 262,
    F90PPR_INCLUDE = 263,
    COCO_INCLUDE = 264,
    F90PPR_DEFINE = 265,
    CPP_DEFINE = 266,
    F90PPR_UNDEF = 267,
    CPP_UNDEF = 268,
    CPP_IFDEF = 269,
    CPP_IFNDEF = 270,
    CPP_IF = 271,
    CPP_ELSE = 272,
    CPP_ELIF = 273,
    CPP_ENDIF = 274,
    F90PPR_IFDEF = 275,
    F90PPR_IFNDEF = 276,
    F90PPR_IF = 277,
    F90PPR_ELSE = 278,
    F90PPR_ELIF = 279,
    F90PPR_ENDIF = 280,
    COMMA = 281,
    COLON = 282,
    DCOLON = 283,
    LPAREN = 284,
    RPAREN = 285,
    UNTERMINATED_STRING = 286,
    STRING = 287,
    WORD = 288,
    CPP_INCLUDE_ANGLE = 289,
    END = 290,
    INCLUDE = 291,
    INTERFACE = 292,
    MODULE = 293,
    SUBMODULE = 294,
    USE = 295
  };
#endif
/* Tokens.  */
#define EOSTMT 258
#define ASSIGNMENT_OP 259
#define GARBAGE 260
#define CPP_LINE_DIRECTIVE 261
#define CPP_INCLUDE 262
#define F90PPR_INCLUDE 263
#define COCO_INCLUDE 264
#define F90PPR_DEFINE 265
#define CPP_DEFINE 266
#define F90PPR_UNDEF 267
#define CPP_UNDEF 268
#define CPP_IFDEF 269
#define CPP_IFNDEF 270
#define CPP_IF 271
#define CPP_ELSE 272
#define CPP_ELIF 273
#define CPP_ENDIF 274
#define F90PPR_IFDEF 275
#define F90PPR_IFNDEF 276
#define F90PPR_IF 277
#define F90PPR_ELSE 278
#define F90PPR_ELIF 279
#define F90PPR_ENDIF 280
#define COMMA 281
#define COLON 282
#define DCOLON 283
#define LPAREN 284
#define RPAREN 285
#define UNTERMINATED_STRING 286
#define STRING 287
#define WORD 288
#define CPP_INCLUDE_ANGLE 289
#define END 290
#define INCLUDE 291
#define INTERFACE 292
#define MODULE 293
#define SUBMODULE 294
#define USE 295

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 70 "cmFortranParser.y" /* yacc.c:1909  */

  char* string;

#line 138 "cmFortranParserTokens.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int cmFortran_yyparse (yyscan_t yyscanner);

#endif /* !YY_CMFORTRAN_YY_CMFORTRANPARSERTOKENS_H_INCLUDED  */
