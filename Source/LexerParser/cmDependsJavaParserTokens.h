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

#ifndef YY_CMDEPENDSJAVA_YY_CMDEPENDSJAVAPARSERTOKENS_H_INCLUDED
# define YY_CMDEPENDSJAVA_YY_CMDEPENDSJAVAPARSERTOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cmDependsJava_yydebug;
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
    jp_ABSTRACT = 258,             /* jp_ABSTRACT  */
    jp_ASSERT = 259,               /* jp_ASSERT  */
    jp_BOOLEAN_TYPE = 260,         /* jp_BOOLEAN_TYPE  */
    jp_BREAK = 261,                /* jp_BREAK  */
    jp_BYTE_TYPE = 262,            /* jp_BYTE_TYPE  */
    jp_CASE = 263,                 /* jp_CASE  */
    jp_CATCH = 264,                /* jp_CATCH  */
    jp_CHAR_TYPE = 265,            /* jp_CHAR_TYPE  */
    jp_CLASS = 266,                /* jp_CLASS  */
    jp_CONTINUE = 267,             /* jp_CONTINUE  */
    jp_DEFAULT = 268,              /* jp_DEFAULT  */
    jp_DO = 269,                   /* jp_DO  */
    jp_DOUBLE_TYPE = 270,          /* jp_DOUBLE_TYPE  */
    jp_ELSE = 271,                 /* jp_ELSE  */
    jp_EXTENDS = 272,              /* jp_EXTENDS  */
    jp_FINAL = 273,                /* jp_FINAL  */
    jp_FINALLY = 274,              /* jp_FINALLY  */
    jp_FLOAT_TYPE = 275,           /* jp_FLOAT_TYPE  */
    jp_FOR = 276,                  /* jp_FOR  */
    jp_IF = 277,                   /* jp_IF  */
    jp_IMPLEMENTS = 278,           /* jp_IMPLEMENTS  */
    jp_IMPORT = 279,               /* jp_IMPORT  */
    jp_INSTANCEOF = 280,           /* jp_INSTANCEOF  */
    jp_INT_TYPE = 281,             /* jp_INT_TYPE  */
    jp_INTERFACE = 282,            /* jp_INTERFACE  */
    jp_LONG_TYPE = 283,            /* jp_LONG_TYPE  */
    jp_NATIVE = 284,               /* jp_NATIVE  */
    jp_NEW = 285,                  /* jp_NEW  */
    jp_PACKAGE = 286,              /* jp_PACKAGE  */
    jp_PRIVATE = 287,              /* jp_PRIVATE  */
    jp_PROTECTED = 288,            /* jp_PROTECTED  */
    jp_PUBLIC = 289,               /* jp_PUBLIC  */
    jp_RETURN = 290,               /* jp_RETURN  */
    jp_SHORT_TYPE = 291,           /* jp_SHORT_TYPE  */
    jp_STATIC = 292,               /* jp_STATIC  */
    jp_STRICTFP = 293,             /* jp_STRICTFP  */
    jp_SUPER = 294,                /* jp_SUPER  */
    jp_SWITCH = 295,               /* jp_SWITCH  */
    jp_SYNCHRONIZED = 296,         /* jp_SYNCHRONIZED  */
    jp_THIS = 297,                 /* jp_THIS  */
    jp_THROW = 298,                /* jp_THROW  */
    jp_THROWS = 299,               /* jp_THROWS  */
    jp_TRANSIENT = 300,            /* jp_TRANSIENT  */
    jp_TRY = 301,                  /* jp_TRY  */
    jp_VOID = 302,                 /* jp_VOID  */
    jp_VOLATILE = 303,             /* jp_VOLATILE  */
    jp_WHILE = 304,                /* jp_WHILE  */
    jp_BOOLEANLITERAL = 305,       /* jp_BOOLEANLITERAL  */
    jp_CHARACTERLITERAL = 306,     /* jp_CHARACTERLITERAL  */
    jp_DECIMALINTEGERLITERAL = 307, /* jp_DECIMALINTEGERLITERAL  */
    jp_FLOATINGPOINTLITERAL = 308, /* jp_FLOATINGPOINTLITERAL  */
    jp_HEXINTEGERLITERAL = 309,    /* jp_HEXINTEGERLITERAL  */
    jp_NULLLITERAL = 310,          /* jp_NULLLITERAL  */
    jp_STRINGLITERAL = 311,        /* jp_STRINGLITERAL  */
    jp_NAME = 312,                 /* jp_NAME  */
    jp_AND = 313,                  /* jp_AND  */
    jp_ANDAND = 314,               /* jp_ANDAND  */
    jp_ANDEQUALS = 315,            /* jp_ANDEQUALS  */
    jp_BRACKETEND = 316,           /* jp_BRACKETEND  */
    jp_BRACKETSTART = 317,         /* jp_BRACKETSTART  */
    jp_CARROT = 318,               /* jp_CARROT  */
    jp_CARROTEQUALS = 319,         /* jp_CARROTEQUALS  */
    jp_COLON = 320,                /* jp_COLON  */
    jp_COMMA = 321,                /* jp_COMMA  */
    jp_CURLYEND = 322,             /* jp_CURLYEND  */
    jp_CURLYSTART = 323,           /* jp_CURLYSTART  */
    jp_DIVIDE = 324,               /* jp_DIVIDE  */
    jp_DIVIDEEQUALS = 325,         /* jp_DIVIDEEQUALS  */
    jp_DOLLAR = 326,               /* jp_DOLLAR  */
    jp_DOT = 327,                  /* jp_DOT  */
    jp_EQUALS = 328,               /* jp_EQUALS  */
    jp_EQUALSEQUALS = 329,         /* jp_EQUALSEQUALS  */
    jp_EXCLAMATION = 330,          /* jp_EXCLAMATION  */
    jp_EXCLAMATIONEQUALS = 331,    /* jp_EXCLAMATIONEQUALS  */
    jp_GREATER = 332,              /* jp_GREATER  */
    jp_GTEQUALS = 333,             /* jp_GTEQUALS  */
    jp_GTGT = 334,                 /* jp_GTGT  */
    jp_GTGTEQUALS = 335,           /* jp_GTGTEQUALS  */
    jp_GTGTGT = 336,               /* jp_GTGTGT  */
    jp_GTGTGTEQUALS = 337,         /* jp_GTGTGTEQUALS  */
    jp_LESLESEQUALS = 338,         /* jp_LESLESEQUALS  */
    jp_LESSTHAN = 339,             /* jp_LESSTHAN  */
    jp_LTEQUALS = 340,             /* jp_LTEQUALS  */
    jp_LTLT = 341,                 /* jp_LTLT  */
    jp_MINUS = 342,                /* jp_MINUS  */
    jp_MINUSEQUALS = 343,          /* jp_MINUSEQUALS  */
    jp_MINUSMINUS = 344,           /* jp_MINUSMINUS  */
    jp_PAREEND = 345,              /* jp_PAREEND  */
    jp_PARESTART = 346,            /* jp_PARESTART  */
    jp_PERCENT = 347,              /* jp_PERCENT  */
    jp_PERCENTEQUALS = 348,        /* jp_PERCENTEQUALS  */
    jp_PIPE = 349,                 /* jp_PIPE  */
    jp_PIPEEQUALS = 350,           /* jp_PIPEEQUALS  */
    jp_PIPEPIPE = 351,             /* jp_PIPEPIPE  */
    jp_PLUS = 352,                 /* jp_PLUS  */
    jp_PLUSEQUALS = 353,           /* jp_PLUSEQUALS  */
    jp_PLUSPLUS = 354,             /* jp_PLUSPLUS  */
    jp_QUESTION = 355,             /* jp_QUESTION  */
    jp_SEMICOL = 356,              /* jp_SEMICOL  */
    jp_TILDE = 357,                /* jp_TILDE  */
    jp_TIMES = 358,                /* jp_TIMES  */
    jp_TIMESEQUALS = 359,          /* jp_TIMESEQUALS  */
    jp_ERROR = 360                 /* jp_ERROR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */




int cmDependsJava_yyparse (yyscan_t yyscanner);


#endif /* !YY_CMDEPENDSJAVA_YY_CMDEPENDSJAVAPARSERTOKENS_H_INCLUDED  */
