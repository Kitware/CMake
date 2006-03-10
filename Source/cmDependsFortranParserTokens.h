/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison, Copyright (C) 1984,
   1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     USE = 258,
     F_INCLUDE = 259,
     MODULE = 260,
     EOSTMT = 261,
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
     UNTERMINATED_STRING = 281,
     CPP_TOENDL = 282,
     STRING = 283,
     WORD = 284
   };
#endif
#define USE 258
#define F_INCLUDE 259
#define MODULE 260
#define EOSTMT 261
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
#define UNTERMINATED_STRING 281
#define CPP_TOENDL 282
#define STRING 283
#define WORD 284




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 75 "cmDependsFortranParser.y"
typedef union YYSTYPE {
  char* string;
} YYSTYPE;
/* Line 1285 of yacc.c.  */
#line 99 "cmDependsFortranParserTokens.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





