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

#ifndef YY_CMCOMMANDARGUMENT_YY_CMCOMMANDARGUMENTPARSERTOKENS_H_INCLUDED
# define YY_CMCOMMANDARGUMENT_YY_CMCOMMANDARGUMENTPARSERTOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cmCommandArgument_yydebug;
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
    cal_ENVCURLY = 258,            /* cal_ENVCURLY  */
    cal_NCURLY = 259,              /* cal_NCURLY  */
    cal_DCURLY = 260,              /* cal_DCURLY  */
    cal_DOLLAR = 261,              /* "$"  */
    cal_LCURLY = 262,              /* "{"  */
    cal_RCURLY = 263,              /* "}"  */
    cal_NAME = 264,                /* cal_NAME  */
    cal_BSLASH = 265,              /* "\\"  */
    cal_SYMBOL = 266,              /* cal_SYMBOL  */
    cal_AT = 267,                  /* "@"  */
    cal_ERROR = 268,               /* cal_ERROR  */
    cal_ATNAME = 269               /* cal_ATNAME  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */




int cmCommandArgument_yyparse (yyscan_t yyscanner);


#endif /* !YY_CMCOMMANDARGUMENT_YY_CMCOMMANDARGUMENTPARSERTOKENS_H_INCLUDED  */
