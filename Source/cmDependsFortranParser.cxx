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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse cmDependsFortran_yyparse
#define yylex   cmDependsFortran_yylex
#define yyerror cmDependsFortran_yyerror
#define yylval  cmDependsFortran_yylval
#define yychar  cmDependsFortran_yychar
#define yydebug cmDependsFortran_yydebug
#define yynerrs cmDependsFortran_yynerrs


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




/* Copy the first part of user declarations.  */
#line 1 "cmDependsFortranParser.y"

/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Portions of this source have been derived from makefdep90 version 2.6.2,

   Copyright (C) 2000,2001 Erik Edelmann <eedelman@beam.helsinki.fi>.

  The code was originally distributed under the GPL but permission
  from the copyright holder has been obtained to distribute this
  derived work under the CMake license.
-------------------------------------------------------------------------*/

/*

This file must be translated to C and modified to build everywhere.

Run bison like this:

  bison --yacc --name-prefix=cmDependsFortran_yy --defines=cmDependsFortranParserTokens.h -ocmDependsFortranParser.cxx cmDependsFortranParser.y

Modify cmDependsFortranParser.cxx:
  - remove TABs

*/

/*-------------------------------------------------------------------------*/
#define cmDependsFortranParser_cxx
#include "cmDependsFortranParser.h" /* Interface to parser object.  */
#include "cmDependsFortranParserTokens.h" /* Need YYSTYPE for YY_DECL.  */

/* Configure the parser to use a lexer object.  */
#define YYPARSE_PARAM yyscanner
#define YYLEX_PARAM yyscanner
#define YYERROR_VERBOSE 1
#define cmDependsFortran_yyerror(x) \
        cmDependsFortranError(yyscanner, x)

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Helper function to forward error callback.  */
static void cmDependsFortranError(yyscan_t yyscanner, const char* message)
{
  cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
  cmDependsFortranParser_Error(parser, message);
}

/* Disable some warnings in the generated code.  */
#ifdef __BORLANDC__
# pragma warn -8004 /* Variable assigned a value that is not used.  */
#endif
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
#endif


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 75 "cmDependsFortranParser.y"
typedef union YYSTYPE {
  char* string;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 218 "cmDependsFortranParser.cxx"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 230 "cmDependsFortranParser.cxx"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
         || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))                     \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)              \
      do                                        \
        {                                       \
          register YYSIZE_T yyi;                \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (To)[yyi] = (From)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)                                        \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack, Stack, yysize);                          \
        Stack = &yyptr->Stack;                                          \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   67

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  15
/* YYNRULES -- Number of rules. */
#define YYNRULES  43
/* YYNRULES -- Number of states. */
#define YYNSTATES  73

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   284

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     7,    12,    17,    22,    26,    31,
      36,    41,    46,    50,    54,    58,    62,    65,    66,    68,
      70,    72,    74,    76,    78,    80,    82,    84,    86,    88,
      90,    92,    94,    96,    98,   100,   102,   104,   106,   108,
     109,   112,   114,   116
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      31,     0,    -1,    -1,    31,    32,    -1,     3,    29,    43,
      33,    -1,    34,    28,    43,    33,    -1,     7,    29,    43,
      33,    -1,     5,    29,    33,    -1,    35,    29,    43,    33,
      -1,    36,    29,    43,    33,    -1,    37,    29,    43,    33,
      -1,    38,    29,    43,    33,    -1,    39,    43,    33,    -1,
      40,    43,    33,    -1,    41,    43,    33,    -1,    42,    43,
      33,    -1,    43,    33,    -1,    -1,     6,    -1,     4,    -1,
       7,    -1,     8,    -1,     9,    -1,    11,    -1,    10,    -1,
      13,    -1,    12,    -1,    14,    -1,    20,    -1,    15,    -1,
      21,    -1,    16,    -1,    22,    -1,    18,    -1,    24,    -1,
      17,    -1,    23,    -1,    19,    -1,    25,    -1,    -1,    43,
      44,    -1,    29,    -1,    28,    -1,    26,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    93,    93,    93,    96,   102,   108,   109,   115,   121,
     127,   133,   139,   144,   149,   154,   159,   162,   162,   163,
     163,   163,   163,   164,   164,   165,   165,   166,   166,   167,
     167,   168,   168,   169,   169,   170,   170,   171,   171,   172,
     172,   175,   176,   177
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "USE", "F_INCLUDE", "MODULE", "EOSTMT",
  "CPP_INCLUDE", "F90PPR_INCLUDE", "COCO_INCLUDE", "F90PPR_DEFINE",
  "CPP_DEFINE", "F90PPR_UNDEF", "CPP_UNDEF", "CPP_IFDEF", "CPP_IFNDEF",
  "CPP_IF", "CPP_ELSE", "CPP_ELIF", "CPP_ENDIF", "F90PPR_IFDEF",
  "F90PPR_IFNDEF", "F90PPR_IF", "F90PPR_ELSE", "F90PPR_ELIF",
  "F90PPR_ENDIF", "UNTERMINATED_STRING", "CPP_TOENDL", "STRING", "WORD",
  "$accept", "code", "stmt", "eostmt", "include", "define", "undef",
  "ifdef", "ifndef", "if", "elif", "else", "endif", "other", "misc_code", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    30,    31,    31,    32,    32,    32,    32,    32,    32,
      32,    32,    32,    32,    32,    32,    32,    33,    33,    34,
      34,    34,    34,    35,    35,    36,    36,    37,    37,    38,
      38,    39,    39,    40,    40,    41,    41,    42,    42,    43,
      43,    44,    44,    44
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     4,     4,     4,     3,     4,     4,
       4,     4,     3,     3,     3,     3,     2,     0,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       2,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,    39,     1,     0,    19,     0,    20,    21,    22,    24,
      23,    26,    25,    27,    29,    31,    35,    33,    37,    28,
      30,    32,    36,    34,    38,     3,     0,     0,     0,     0,
       0,    39,    39,    39,    39,    17,    39,    17,    39,    39,
      39,    39,    39,    39,    17,    17,    17,    17,    18,    43,
      42,    41,    16,    40,    17,     7,    17,    17,    17,    17,
      17,    17,    12,    13,    14,    15,     4,     6,     5,     8,
       9,    10,    11
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,    25,    52,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    53
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -29
static const yysigned_char yypact[] =
{
     -29,     0,   -29,   -28,   -29,   -27,   -23,   -29,   -29,   -29,
     -29,   -29,   -29,   -29,   -29,   -29,   -29,   -29,   -29,   -29,
     -29,   -29,   -29,   -29,   -29,   -29,    -1,     1,     3,     8,
       9,   -29,   -29,   -29,   -29,    25,   -29,    22,   -29,   -29,
     -29,   -29,   -29,   -29,    25,    25,    25,    25,   -29,   -29,
     -29,   -29,   -29,   -29,    25,   -29,    25,    25,    25,    25,
      25,    25,   -29,   -29,   -29,   -29,   -29,   -29,   -29,   -29,
     -29,   -29,   -29
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -29,   -29,   -29,   -11,   -29,   -29,   -29,   -29,   -29,   -29,
     -29,   -29,   -29,    24,   -29
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
       2,    36,    37,     3,     4,     5,    38,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    55,    39,    48,     0,
      40,    48,    41,    62,    63,    64,    65,    42,    43,     0,
       0,     0,     0,    66,     0,    67,    68,    69,    70,    71,
      72,    49,     0,    50,    51,    44,    45,    46,    47,     0,
      54,     0,    56,    57,    58,    59,    60,    61
};

static const yysigned_char yycheck[] =
{
       0,    29,    29,     3,     4,     5,    29,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    37,    28,     6,    -1,
      29,     6,    29,    44,    45,    46,    47,    29,    29,    -1,
      -1,    -1,    -1,    54,    -1,    56,    57,    58,    59,    60,
      61,    26,    -1,    28,    29,    31,    32,    33,    34,    -1,
      36,    -1,    38,    39,    40,    41,    42,    43
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    31,     0,     3,     4,     5,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    32,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    29,    29,    29,    28,
      29,    29,    29,    29,    43,    43,    43,    43,     6,    26,
      28,    29,    33,    44,    43,    33,    43,    43,    43,    43,
      43,    43,    33,    33,    33,    33,    33,    33,    33,    33,
      33,    33,    33
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL          goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY && yylen == 1)                          \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      yytoken = YYTRANSLATE (yychar);                           \
      YYPOPSTACK;                                               \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror ("syntax error: cannot back up");\
      YYERROR;                                                  \
    }                                                           \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                \
   ((Current).first_line   = (Rhs)[1].first_line,       \
    (Current).first_column = (Rhs)[1].first_column,     \
    (Current).last_line    = (Rhs)[N].last_line,        \
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

# define YYDSYMPRINT(Args)                      \
do {                                            \
  if (yydebug)                                  \
    yysymprint Args;                            \
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)            \
do {                                                            \
  if (yydebug)                                                  \
    {                                                           \
      YYFPRINTF (stderr, "%s ", Title);                         \
      yysymprint (stderr,                                       \
                  Token, Value);        \
      YYFPRINTF (stderr, "\n");                                 \
    }                                                           \
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (Rule);             \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  register short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;             /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;


  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack. Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        short int *yyss1 = yyss;


        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow ("parser stack overflow",
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),

                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        short int *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyoverflowlab;
        YYSTACK_RELOCATE (yyss);
        YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 97 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleUse(parser, yyvsp[-2].string);
    free(yyvsp[-2].string);
    }
    break;

  case 5:
#line 103 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleInclude(parser, yyvsp[-2].string);
    free(yyvsp[-2].string);
    }
    break;

  case 7:
#line 110 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleModule(parser, yyvsp[-1].string);
    free(yyvsp[-1].string);
    }
    break;

  case 8:
#line 116 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleDefine(parser, yyvsp[-2].string);
    free(yyvsp[-2].string);
    }
    break;

  case 9:
#line 122 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleUndef(parser, yyvsp[-2].string);
    free(yyvsp[-2].string);
    }
    break;

  case 10:
#line 128 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfdef(parser, yyvsp[-2].string);
    free(yyvsp[-2].string);
    }
    break;

  case 11:
#line 134 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIfndef(parser, yyvsp[-2].string);
    free(yyvsp[-2].string);
    }
    break;

  case 12:
#line 140 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleIf(parser);
    }
    break;

  case 13:
#line 145 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElif(parser);
    }
    break;

  case 14:
#line 150 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleElse(parser);
    }
    break;

  case 15:
#line 155 "cmDependsFortranParser.y"
    {
    cmDependsFortranParser* parser = cmDependsFortran_yyget_extra(yyscanner);
    cmDependsFortranParser_RuleEndif(parser);
    }
    break;

  case 41:
#line 175 "cmDependsFortranParser.y"
    { free (yyvsp[0].string); }
    break;

  case 42:
#line 176 "cmDependsFortranParser.y"
    { free (yyvsp[0].string); }
    break;


    }

/* Line 1010 of yacc.c.  */
#line 1277 "cmDependsFortranParser.cxx"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
        {
          YYSIZE_T yysize = 0;
          int yytype = YYTRANSLATE (yychar);
          const char* yyprefix;
          char *yymsg;
          int yyx;

          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;

          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yycount = 0;

          yyprefix = ", expecting ";
          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
              {
                yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
                yycount += 1;
                if (yycount == 5)
                  {
                    yysize = 0;
                    break;
                  }
              }
          yysize += (sizeof ("syntax error, unexpected ")
                     + yystrlen (yytname[yytype]));
          yymsg = (char *) YYSTACK_ALLOC (yysize);
          if (yymsg != 0)
            {
              char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
              yyp = yystpcpy (yyp, yytname[yytype]);

              if (yycount < 5)
                {
                  yyprefix = ", expecting ";
                  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
                    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
                      {
                        yyp = yystpcpy (yyp, yyprefix);
                        yyp = yystpcpy (yyp, yytname[yyx]);
                        yyprefix = " or ";
                      }
                }
              yyerror (yymsg);
              YYSTACK_FREE (yymsg);
            }
          else
            yyerror ("syntax error; also virtual memory exhausted");
        }
      else
#endif /* YYERROR_VERBOSE */
        yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
             then the rest of the stack, then return failure.  */
          if (yychar == YYEOF)
             for (;;)
               {
                 YYPOPSTACK;
                 if (yyssp == yyss)
                   YYABORT;
                 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
                 yydestruct (yystos[*yyssp], yyvsp);
               }
        }
      else
        {
          YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
          yydestruct (yytoken, &yylval);
          yychar = YYEMPTY;

        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 180 "cmDependsFortranParser.y"

/* End of grammar */

