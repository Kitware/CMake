/* A Bison parser, made by GNU Bison 3.4.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_CMEXPR_YY_CMEXPRPARSERTOKENS_H_INCLUDED
# define YY_CMEXPR_YY_CMEXPRPARSERTOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cmExpr_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    exp_PLUS = 258,
    exp_MINUS = 259,
    exp_TIMES = 260,
    exp_DIVIDE = 261,
    exp_MOD = 262,
    exp_SHIFTLEFT = 263,
    exp_SHIFTRIGHT = 264,
    exp_OPENPARENT = 265,
    exp_CLOSEPARENT = 266,
    exp_OR = 267,
    exp_AND = 268,
    exp_XOR = 269,
    exp_NOT = 270,
    exp_NUMBER = 271
  };
#endif
/* Tokens.  */
#define exp_PLUS 258
#define exp_MINUS 259
#define exp_TIMES 260
#define exp_DIVIDE 261
#define exp_MOD 262
#define exp_SHIFTLEFT 263
#define exp_SHIFTRIGHT 264
#define exp_OPENPARENT 265
#define exp_CLOSEPARENT 266
#define exp_OR 267
#define exp_AND 268
#define exp_XOR 269
#define exp_NOT 270
#define exp_NUMBER 271

/* Value type.  */



int cmExpr_yyparse (yyscan_t yyscanner);

#endif /* !YY_CMEXPR_YY_CMEXPRPARSERTOKENS_H_INCLUDED  */
