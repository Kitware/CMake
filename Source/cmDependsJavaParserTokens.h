/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
     ABSTRACT = 258,
     ASSERT = 259,
     BOOLEAN_TYPE = 260,
     BREAK = 261,
     BYTE_TYPE = 262,
     CASE = 263,
     CATCH = 264,
     CHAR_TYPE = 265,
     CLASS = 266,
     CONTINUE = 267,
     DEFAULT = 268,
     DO = 269,
     DOUBLE_TYPE = 270,
     ELSE = 271,
     EXTENDS = 272,
     FINAL = 273,
     FINALLY = 274,
     FLOAT_TYPE = 275,
     FOR = 276,
     IF = 277,
     IMPLEMENTS = 278,
     IMPORT = 279,
     INSTANCEOF = 280,
     INT_TYPE = 281,
     INTERFACE = 282,
     LONG_TYPE = 283,
     NATIVE = 284,
     NEW = 285,
     PACKAGE = 286,
     PRIVATE = 287,
     PROTECTED = 288,
     PUBLIC = 289,
     RETURN = 290,
     SHORT_TYPE = 291,
     STATIC = 292,
     STRICTFP = 293,
     SUPER = 294,
     SWITCH = 295,
     SYNCHRONIZED = 296,
     THIS = 297,
     THROW = 298,
     THROWS = 299,
     TRANSIENT = 300,
     TRY = 301,
     VOID = 302,
     VOLATILE = 303,
     WHILE = 304,
     BOOLEANLITERAL = 305,
     CHARACTERLITERAL = 306,
     DECIMALINTEGERLITERAL = 307,
     FLOATINGPOINTLITERAL = 308,
     HEXINTEGERLITERAL = 309,
     NULLLITERAL = 310,
     STRINGLITERAL = 311,
     NAME = 312,
     AND = 313,
     ANDAND = 314,
     ANDEQUALS = 315,
     BRACKETEND = 316,
     BRACKETSTART = 317,
     CARROT = 318,
     CARROTEQUALS = 319,
     COLON = 320,
     COMMA = 321,
     CURLYEND = 322,
     CURLYSTART = 323,
     DIVIDE = 324,
     DIVIDEEQUALS = 325,
     DOLLAR = 326,
     DOT = 327,
     EQUALS = 328,
     EQUALSEQUALS = 329,
     EXCLAMATION = 330,
     EXCLAMATIONEQUALS = 331,
     GREATER = 332,
     GTEQUALS = 333,
     GTGT = 334,
     GTGTEQUALS = 335,
     GTGTGT = 336,
     GTGTGTEQUALS = 337,
     LESLESEQUALS = 338,
     LESSTHAN = 339,
     LTEQUALS = 340,
     LTLT = 341,
     MINUS = 342,
     MINUSEQUALS = 343,
     MINUSMINUS = 344,
     PAREEND = 345,
     PARESTART = 346,
     PERCENT = 347,
     PERCENTEQUALS = 348,
     PIPE = 349,
     PIPEEQUALS = 350,
     PIPEPIPE = 351,
     PLUS = 352,
     PLUSEQUALS = 353,
     PLUSPLUS = 354,
     QUESTION = 355,
     SEMICOL = 356,
     TILDE = 357,
     TIMES = 358,
     TIMESEQUALS = 359,
     ERROR = 360
   };
#endif
#define ABSTRACT 258
#define ASSERT 259
#define BOOLEAN_TYPE 260
#define BREAK 261
#define BYTE_TYPE 262
#define CASE 263
#define CATCH 264
#define CHAR_TYPE 265
#define CLASS 266
#define CONTINUE 267
#define DEFAULT 268
#define DO 269
#define DOUBLE_TYPE 270
#define ELSE 271
#define EXTENDS 272
#define FINAL 273
#define FINALLY 274
#define FLOAT_TYPE 275
#define FOR 276
#define IF 277
#define IMPLEMENTS 278
#define IMPORT 279
#define INSTANCEOF 280
#define INT_TYPE 281
#define INTERFACE 282
#define LONG_TYPE 283
#define NATIVE 284
#define NEW 285
#define PACKAGE 286
#define PRIVATE 287
#define PROTECTED 288
#define PUBLIC 289
#define RETURN 290
#define SHORT_TYPE 291
#define STATIC 292
#define STRICTFP 293
#define SUPER 294
#define SWITCH 295
#define SYNCHRONIZED 296
#define THIS 297
#define THROW 298
#define THROWS 299
#define TRANSIENT 300
#define TRY 301
#define VOID 302
#define VOLATILE 303
#define WHILE 304
#define BOOLEANLITERAL 305
#define CHARACTERLITERAL 306
#define DECIMALINTEGERLITERAL 307
#define FLOATINGPOINTLITERAL 308
#define HEXINTEGERLITERAL 309
#define NULLLITERAL 310
#define STRINGLITERAL 311
#define NAME 312
#define AND 313
#define ANDAND 314
#define ANDEQUALS 315
#define BRACKETEND 316
#define BRACKETSTART 317
#define CARROT 318
#define CARROTEQUALS 319
#define COLON 320
#define COMMA 321
#define CURLYEND 322
#define CURLYSTART 323
#define DIVIDE 324
#define DIVIDEEQUALS 325
#define DOLLAR 326
#define DOT 327
#define EQUALS 328
#define EQUALSEQUALS 329
#define EXCLAMATION 330
#define EXCLAMATIONEQUALS 331
#define GREATER 332
#define GTEQUALS 333
#define GTGT 334
#define GTGTEQUALS 335
#define GTGTGT 336
#define GTGTGTEQUALS 337
#define LESLESEQUALS 338
#define LESSTHAN 339
#define LTEQUALS 340
#define LTLT 341
#define MINUS 342
#define MINUSEQUALS 343
#define MINUSMINUS 344
#define PAREEND 345
#define PARESTART 346
#define PERCENT 347
#define PERCENTEQUALS 348
#define PIPE 349
#define PIPEEQUALS 350
#define PIPEPIPE 351
#define PLUS 352
#define PLUSEQUALS 353
#define PLUSPLUS 354
#define QUESTION 355
#define SEMICOL 356
#define TILDE 357
#define TIMES 358
#define TIMESEQUALS 359
#define ERROR 360




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





