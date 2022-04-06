/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_CMFORTRAN_YY_CMFORTRANPARSERTOKENS_H_INCLUDED
# define YY_CMFORTRAN_YY_CMFORTRANPARSERTOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cmFortran_yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    EOSTMT = 258,                  /* EOSTMT  */
    ASSIGNMENT_OP = 259,           /* ASSIGNMENT_OP  */
    GARBAGE = 260,                 /* GARBAGE  */
    CPP_LINE_DIRECTIVE = 261,      /* CPP_LINE_DIRECTIVE  */
    CPP_INCLUDE = 262,             /* CPP_INCLUDE  */
    F90PPR_INCLUDE = 263,          /* F90PPR_INCLUDE  */
    COCO_INCLUDE = 264,            /* COCO_INCLUDE  */
    F90PPR_DEFINE = 265,           /* F90PPR_DEFINE  */
    CPP_DEFINE = 266,              /* CPP_DEFINE  */
    F90PPR_UNDEF = 267,            /* F90PPR_UNDEF  */
    CPP_UNDEF = 268,               /* CPP_UNDEF  */
    CPP_IFDEF = 269,               /* CPP_IFDEF  */
    CPP_IFNDEF = 270,              /* CPP_IFNDEF  */
    CPP_IF = 271,                  /* CPP_IF  */
    CPP_ELSE = 272,                /* CPP_ELSE  */
    CPP_ELIF = 273,                /* CPP_ELIF  */
    CPP_ENDIF = 274,               /* CPP_ENDIF  */
    F90PPR_IFDEF = 275,            /* F90PPR_IFDEF  */
    F90PPR_IFNDEF = 276,           /* F90PPR_IFNDEF  */
    F90PPR_IF = 277,               /* F90PPR_IF  */
    F90PPR_ELSE = 278,             /* F90PPR_ELSE  */
    F90PPR_ELIF = 279,             /* F90PPR_ELIF  */
    F90PPR_ENDIF = 280,            /* F90PPR_ENDIF  */
    COMMA = 281,                   /* COMMA  */
    COLON = 282,                   /* COLON  */
    DCOLON = 283,                  /* DCOLON  */
    LPAREN = 284,                  /* LPAREN  */
    RPAREN = 285,                  /* RPAREN  */
    UNTERMINATED_STRING = 286,     /* UNTERMINATED_STRING  */
    STRING = 287,                  /* STRING  */
    WORD = 288,                    /* WORD  */
    CPP_INCLUDE_ANGLE = 289,       /* CPP_INCLUDE_ANGLE  */
    END = 290,                     /* END  */
    INCLUDE = 291,                 /* INCLUDE  */
    INTERFACE = 292,               /* INTERFACE  */
    MODULE = 293,                  /* MODULE  */
    SUBMODULE = 294,               /* SUBMODULE  */
    USE = 295                      /* USE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 76 "cmFortranParser.y"

  char* string;

#line 108 "cmFortranParserTokens.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int cmFortran_yyparse (yyscan_t yyscanner);


#endif /* !YY_CMFORTRAN_YY_CMFORTRANPARSERTOKENS_H_INCLUDED  */
