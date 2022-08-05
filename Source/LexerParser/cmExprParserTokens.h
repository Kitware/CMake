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

#ifndef YY_CMEXPR_YY_CMEXPRPARSERTOKENS_H_INCLUDED
# define YY_CMEXPR_YY_CMEXPRPARSERTOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cmExpr_yydebug;
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
    exp_PLUS = 258,                /* exp_PLUS  */
    exp_MINUS = 259,               /* exp_MINUS  */
    exp_TIMES = 260,               /* exp_TIMES  */
    exp_DIVIDE = 261,              /* exp_DIVIDE  */
    exp_MOD = 262,                 /* exp_MOD  */
    exp_SHIFTLEFT = 263,           /* exp_SHIFTLEFT  */
    exp_SHIFTRIGHT = 264,          /* exp_SHIFTRIGHT  */
    exp_OPENPARENT = 265,          /* exp_OPENPARENT  */
    exp_CLOSEPARENT = 266,         /* exp_CLOSEPARENT  */
    exp_OR = 267,                  /* exp_OR  */
    exp_AND = 268,                 /* exp_AND  */
    exp_XOR = 269,                 /* exp_XOR  */
    exp_NOT = 270,                 /* exp_NOT  */
    exp_NUMBER = 271               /* exp_NUMBER  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */




int cmExpr_yyparse (yyscan_t yyscanner);


#endif /* !YY_CMEXPR_YY_CMEXPRPARSERTOKENS_H_INCLUDED  */
