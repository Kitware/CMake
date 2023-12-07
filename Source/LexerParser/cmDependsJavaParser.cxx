/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         cmDependsJava_yyparse
#define yylex           cmDependsJava_yylex
#define yyerror         cmDependsJava_yyerror
#define yydebug         cmDependsJava_yydebug
#define yynerrs         cmDependsJava_yynerrs

/* First part of user prologue.  */
#line 1 "cmDependsJavaParser.y"

/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/*

This file must be translated to C and modified to build everywhere.

Run bison like this:

  bison --name-prefix=cmDependsJava_yy --defines=cmDependsJavaParserTokens.h -ocmDependsJavaParser.cxx cmDependsJavaParser.y

*/

#include "cmConfigure.h" // IWYU pragma: keep

#include <stdlib.h>
#include <string.h>
#include <string>

#define yyGetParser (cmDependsJava_yyget_extra(yyscanner))

/*-------------------------------------------------------------------------*/
#include "cmDependsJavaParserHelper.h" /* Interface to parser object.  */
#include "cmDependsJavaLexer.h"  /* Interface to lexer object.  */

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Helper function to forward error callback from parser.  */
static void cmDependsJava_yyerror(yyscan_t yyscanner, const char* message);

#define YYMAXDEPTH 1000000


#define jpCheckEmpty(cnt) yyGetParser->CheckEmpty(__LINE__, cnt, yyvsp)
#define jpElementStart(cnt) yyGetParser->PrepareElement(&yyval)
#define jpStoreClass(str) yyGetParser->AddClassFound(str); yyGetParser->DeallocateParserType(&(str))
/* Disable some warnings in the generated code.  */
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
# pragma warning (disable: 4065) /* Switch statement contains default but no case. */
#endif
#if defined(__GNUC__) && __GNUC__ >= 8
# pragma GCC diagnostic ignored "-Wconversion"
# pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif
#if defined(__clang__) && defined(__has_warning)
# if __has_warning("-Wunused-but-set-variable")
#  pragma clang diagnostic ignored "-Wunused-but-set-variable"
# endif
#endif

#line 129 "cmDependsJavaParser.cxx"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "cmDependsJavaParserTokens.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_jp_ABSTRACT = 3,                /* jp_ABSTRACT  */
  YYSYMBOL_jp_ASSERT = 4,                  /* jp_ASSERT  */
  YYSYMBOL_jp_BOOLEAN_TYPE = 5,            /* jp_BOOLEAN_TYPE  */
  YYSYMBOL_jp_BREAK = 6,                   /* jp_BREAK  */
  YYSYMBOL_jp_BYTE_TYPE = 7,               /* jp_BYTE_TYPE  */
  YYSYMBOL_jp_CASE = 8,                    /* jp_CASE  */
  YYSYMBOL_jp_CATCH = 9,                   /* jp_CATCH  */
  YYSYMBOL_jp_CHAR_TYPE = 10,              /* jp_CHAR_TYPE  */
  YYSYMBOL_jp_CLASS = 11,                  /* jp_CLASS  */
  YYSYMBOL_jp_CONTINUE = 12,               /* jp_CONTINUE  */
  YYSYMBOL_jp_DEFAULT = 13,                /* jp_DEFAULT  */
  YYSYMBOL_jp_DO = 14,                     /* jp_DO  */
  YYSYMBOL_jp_DOUBLE_TYPE = 15,            /* jp_DOUBLE_TYPE  */
  YYSYMBOL_jp_ELSE = 16,                   /* jp_ELSE  */
  YYSYMBOL_jp_EXTENDS = 17,                /* jp_EXTENDS  */
  YYSYMBOL_jp_FINAL = 18,                  /* jp_FINAL  */
  YYSYMBOL_jp_FINALLY = 19,                /* jp_FINALLY  */
  YYSYMBOL_jp_FLOAT_TYPE = 20,             /* jp_FLOAT_TYPE  */
  YYSYMBOL_jp_FOR = 21,                    /* jp_FOR  */
  YYSYMBOL_jp_IF = 22,                     /* jp_IF  */
  YYSYMBOL_jp_IMPLEMENTS = 23,             /* jp_IMPLEMENTS  */
  YYSYMBOL_jp_IMPORT = 24,                 /* jp_IMPORT  */
  YYSYMBOL_jp_INSTANCEOF = 25,             /* jp_INSTANCEOF  */
  YYSYMBOL_jp_INT_TYPE = 26,               /* jp_INT_TYPE  */
  YYSYMBOL_jp_INTERFACE = 27,              /* jp_INTERFACE  */
  YYSYMBOL_jp_LONG_TYPE = 28,              /* jp_LONG_TYPE  */
  YYSYMBOL_jp_NATIVE = 29,                 /* jp_NATIVE  */
  YYSYMBOL_jp_NEW = 30,                    /* jp_NEW  */
  YYSYMBOL_jp_PACKAGE = 31,                /* jp_PACKAGE  */
  YYSYMBOL_jp_PRIVATE = 32,                /* jp_PRIVATE  */
  YYSYMBOL_jp_PROTECTED = 33,              /* jp_PROTECTED  */
  YYSYMBOL_jp_PUBLIC = 34,                 /* jp_PUBLIC  */
  YYSYMBOL_jp_RETURN = 35,                 /* jp_RETURN  */
  YYSYMBOL_jp_SHORT_TYPE = 36,             /* jp_SHORT_TYPE  */
  YYSYMBOL_jp_STATIC = 37,                 /* jp_STATIC  */
  YYSYMBOL_jp_STRICTFP = 38,               /* jp_STRICTFP  */
  YYSYMBOL_jp_SUPER = 39,                  /* jp_SUPER  */
  YYSYMBOL_jp_SWITCH = 40,                 /* jp_SWITCH  */
  YYSYMBOL_jp_SYNCHRONIZED = 41,           /* jp_SYNCHRONIZED  */
  YYSYMBOL_jp_THIS = 42,                   /* jp_THIS  */
  YYSYMBOL_jp_THROW = 43,                  /* jp_THROW  */
  YYSYMBOL_jp_THROWS = 44,                 /* jp_THROWS  */
  YYSYMBOL_jp_TRANSIENT = 45,              /* jp_TRANSIENT  */
  YYSYMBOL_jp_TRY = 46,                    /* jp_TRY  */
  YYSYMBOL_jp_VOID = 47,                   /* jp_VOID  */
  YYSYMBOL_jp_VOLATILE = 48,               /* jp_VOLATILE  */
  YYSYMBOL_jp_WHILE = 49,                  /* jp_WHILE  */
  YYSYMBOL_jp_BOOLEANLITERAL = 50,         /* jp_BOOLEANLITERAL  */
  YYSYMBOL_jp_CHARACTERLITERAL = 51,       /* jp_CHARACTERLITERAL  */
  YYSYMBOL_jp_DECIMALINTEGERLITERAL = 52,  /* jp_DECIMALINTEGERLITERAL  */
  YYSYMBOL_jp_FLOATINGPOINTLITERAL = 53,   /* jp_FLOATINGPOINTLITERAL  */
  YYSYMBOL_jp_HEXINTEGERLITERAL = 54,      /* jp_HEXINTEGERLITERAL  */
  YYSYMBOL_jp_NULLLITERAL = 55,            /* jp_NULLLITERAL  */
  YYSYMBOL_jp_STRINGLITERAL = 56,          /* jp_STRINGLITERAL  */
  YYSYMBOL_jp_NAME = 57,                   /* jp_NAME  */
  YYSYMBOL_jp_AND = 58,                    /* jp_AND  */
  YYSYMBOL_jp_ANDAND = 59,                 /* jp_ANDAND  */
  YYSYMBOL_jp_ANDEQUALS = 60,              /* jp_ANDEQUALS  */
  YYSYMBOL_jp_BRACKETEND = 61,             /* jp_BRACKETEND  */
  YYSYMBOL_jp_BRACKETSTART = 62,           /* jp_BRACKETSTART  */
  YYSYMBOL_jp_CARROT = 63,                 /* jp_CARROT  */
  YYSYMBOL_jp_CARROTEQUALS = 64,           /* jp_CARROTEQUALS  */
  YYSYMBOL_jp_COLON = 65,                  /* jp_COLON  */
  YYSYMBOL_jp_COMMA = 66,                  /* jp_COMMA  */
  YYSYMBOL_jp_CURLYEND = 67,               /* jp_CURLYEND  */
  YYSYMBOL_jp_CURLYSTART = 68,             /* jp_CURLYSTART  */
  YYSYMBOL_jp_DIVIDE = 69,                 /* jp_DIVIDE  */
  YYSYMBOL_jp_DIVIDEEQUALS = 70,           /* jp_DIVIDEEQUALS  */
  YYSYMBOL_jp_DOLLAR = 71,                 /* jp_DOLLAR  */
  YYSYMBOL_jp_DOT = 72,                    /* jp_DOT  */
  YYSYMBOL_jp_EQUALS = 73,                 /* jp_EQUALS  */
  YYSYMBOL_jp_EQUALSEQUALS = 74,           /* jp_EQUALSEQUALS  */
  YYSYMBOL_jp_EXCLAMATION = 75,            /* jp_EXCLAMATION  */
  YYSYMBOL_jp_EXCLAMATIONEQUALS = 76,      /* jp_EXCLAMATIONEQUALS  */
  YYSYMBOL_jp_GREATER = 77,                /* jp_GREATER  */
  YYSYMBOL_jp_GTEQUALS = 78,               /* jp_GTEQUALS  */
  YYSYMBOL_jp_GTGT = 79,                   /* jp_GTGT  */
  YYSYMBOL_jp_GTGTEQUALS = 80,             /* jp_GTGTEQUALS  */
  YYSYMBOL_jp_GTGTGT = 81,                 /* jp_GTGTGT  */
  YYSYMBOL_jp_GTGTGTEQUALS = 82,           /* jp_GTGTGTEQUALS  */
  YYSYMBOL_jp_LESLESEQUALS = 83,           /* jp_LESLESEQUALS  */
  YYSYMBOL_jp_LESSTHAN = 84,               /* jp_LESSTHAN  */
  YYSYMBOL_jp_LTEQUALS = 85,               /* jp_LTEQUALS  */
  YYSYMBOL_jp_LTLT = 86,                   /* jp_LTLT  */
  YYSYMBOL_jp_MINUS = 87,                  /* jp_MINUS  */
  YYSYMBOL_jp_MINUSEQUALS = 88,            /* jp_MINUSEQUALS  */
  YYSYMBOL_jp_MINUSMINUS = 89,             /* jp_MINUSMINUS  */
  YYSYMBOL_jp_PAREEND = 90,                /* jp_PAREEND  */
  YYSYMBOL_jp_PARESTART = 91,              /* jp_PARESTART  */
  YYSYMBOL_jp_PERCENT = 92,                /* jp_PERCENT  */
  YYSYMBOL_jp_PERCENTEQUALS = 93,          /* jp_PERCENTEQUALS  */
  YYSYMBOL_jp_PIPE = 94,                   /* jp_PIPE  */
  YYSYMBOL_jp_PIPEEQUALS = 95,             /* jp_PIPEEQUALS  */
  YYSYMBOL_jp_PIPEPIPE = 96,               /* jp_PIPEPIPE  */
  YYSYMBOL_jp_PLUS = 97,                   /* jp_PLUS  */
  YYSYMBOL_jp_PLUSEQUALS = 98,             /* jp_PLUSEQUALS  */
  YYSYMBOL_jp_PLUSPLUS = 99,               /* jp_PLUSPLUS  */
  YYSYMBOL_jp_QUESTION = 100,              /* jp_QUESTION  */
  YYSYMBOL_jp_SEMICOL = 101,               /* jp_SEMICOL  */
  YYSYMBOL_jp_TILDE = 102,                 /* jp_TILDE  */
  YYSYMBOL_jp_TIMES = 103,                 /* jp_TIMES  */
  YYSYMBOL_jp_TIMESEQUALS = 104,           /* jp_TIMESEQUALS  */
  YYSYMBOL_jp_ERROR = 105,                 /* jp_ERROR  */
  YYSYMBOL_YYACCEPT = 106,                 /* $accept  */
  YYSYMBOL_Goal = 107,                     /* Goal  */
  YYSYMBOL_Literal = 108,                  /* Literal  */
  YYSYMBOL_IntegerLiteral = 109,           /* IntegerLiteral  */
  YYSYMBOL_Type = 110,                     /* Type  */
  YYSYMBOL_PrimitiveType = 111,            /* PrimitiveType  */
  YYSYMBOL_ReferenceType = 112,            /* ReferenceType  */
  YYSYMBOL_ClassOrInterfaceType = 113,     /* ClassOrInterfaceType  */
  YYSYMBOL_ClassType = 114,                /* ClassType  */
  YYSYMBOL_InterfaceType = 115,            /* InterfaceType  */
  YYSYMBOL_ArrayType = 116,                /* ArrayType  */
  YYSYMBOL_Name = 117,                     /* Name  */
  YYSYMBOL_SimpleName = 118,               /* SimpleName  */
  YYSYMBOL_Identifier = 119,               /* Identifier  */
  YYSYMBOL_QualifiedName = 120,            /* QualifiedName  */
  YYSYMBOL_SimpleType = 121,               /* SimpleType  */
  YYSYMBOL_CompilationUnit = 122,          /* CompilationUnit  */
  YYSYMBOL_PackageDeclarationopt = 123,    /* PackageDeclarationopt  */
  YYSYMBOL_ImportDeclarations = 124,       /* ImportDeclarations  */
  YYSYMBOL_TypeDeclarations = 125,         /* TypeDeclarations  */
  YYSYMBOL_PackageDeclaration = 126,       /* PackageDeclaration  */
  YYSYMBOL_ImportDeclaration = 127,        /* ImportDeclaration  */
  YYSYMBOL_SingleTypeImportDeclaration = 128, /* SingleTypeImportDeclaration  */
  YYSYMBOL_TypeImportOnDemandDeclaration = 129, /* TypeImportOnDemandDeclaration  */
  YYSYMBOL_TypeDeclaration = 130,          /* TypeDeclaration  */
  YYSYMBOL_Modifiers = 131,                /* Modifiers  */
  YYSYMBOL_Modifier = 132,                 /* Modifier  */
  YYSYMBOL_ClassHeader = 133,              /* ClassHeader  */
  YYSYMBOL_ClassDeclaration = 134,         /* ClassDeclaration  */
  YYSYMBOL_Modifiersopt = 135,             /* Modifiersopt  */
  YYSYMBOL_Super = 136,                    /* Super  */
  YYSYMBOL_Interfaces = 137,               /* Interfaces  */
  YYSYMBOL_InterfaceTypeList = 138,        /* InterfaceTypeList  */
  YYSYMBOL_ClassBody = 139,                /* ClassBody  */
  YYSYMBOL_ClassBodyDeclarations = 140,    /* ClassBodyDeclarations  */
  YYSYMBOL_ClassBodyDeclaration = 141,     /* ClassBodyDeclaration  */
  YYSYMBOL_ClassMemberDeclaration = 142,   /* ClassMemberDeclaration  */
  YYSYMBOL_FieldDeclaration = 143,         /* FieldDeclaration  */
  YYSYMBOL_VariableDeclarators = 144,      /* VariableDeclarators  */
  YYSYMBOL_VariableDeclarator = 145,       /* VariableDeclarator  */
  YYSYMBOL_VariableDeclaratorId = 146,     /* VariableDeclaratorId  */
  YYSYMBOL_VariableInitializer = 147,      /* VariableInitializer  */
  YYSYMBOL_MethodDeclaration = 148,        /* MethodDeclaration  */
  YYSYMBOL_MethodHeader = 149,             /* MethodHeader  */
  YYSYMBOL_Throwsopt = 150,                /* Throwsopt  */
  YYSYMBOL_MethodDeclarator = 151,         /* MethodDeclarator  */
  YYSYMBOL_FormalParameterListopt = 152,   /* FormalParameterListopt  */
  YYSYMBOL_FormalParameterList = 153,      /* FormalParameterList  */
  YYSYMBOL_FormalParameter = 154,          /* FormalParameter  */
  YYSYMBOL_Throws = 155,                   /* Throws  */
  YYSYMBOL_ClassTypeList = 156,            /* ClassTypeList  */
  YYSYMBOL_MethodBody = 157,               /* MethodBody  */
  YYSYMBOL_StaticInitializer = 158,        /* StaticInitializer  */
  YYSYMBOL_ConstructorDeclaration = 159,   /* ConstructorDeclaration  */
  YYSYMBOL_ConstructorDeclarator = 160,    /* ConstructorDeclarator  */
  YYSYMBOL_ConstructorBody = 161,          /* ConstructorBody  */
  YYSYMBOL_ExplicitConstructorInvocationopt = 162, /* ExplicitConstructorInvocationopt  */
  YYSYMBOL_ExplicitConstructorInvocation = 163, /* ExplicitConstructorInvocation  */
  YYSYMBOL_InterfaceHeader = 164,          /* InterfaceHeader  */
  YYSYMBOL_InterfaceDeclaration = 165,     /* InterfaceDeclaration  */
  YYSYMBOL_ExtendsInterfacesopt = 166,     /* ExtendsInterfacesopt  */
  YYSYMBOL_ExtendsInterfaces = 167,        /* ExtendsInterfaces  */
  YYSYMBOL_InterfaceBody = 168,            /* InterfaceBody  */
  YYSYMBOL_InterfaceMemberDeclarations = 169, /* InterfaceMemberDeclarations  */
  YYSYMBOL_InterfaceMemberDeclaration = 170, /* InterfaceMemberDeclaration  */
  YYSYMBOL_ConstantDeclaration = 171,      /* ConstantDeclaration  */
  YYSYMBOL_AbstractMethodDeclaration = 172, /* AbstractMethodDeclaration  */
  YYSYMBOL_Semicols = 173,                 /* Semicols  */
  YYSYMBOL_ArrayInitializer = 174,         /* ArrayInitializer  */
  YYSYMBOL_VariableInitializersOptional = 175, /* VariableInitializersOptional  */
  YYSYMBOL_VariableInitializers = 176,     /* VariableInitializers  */
  YYSYMBOL_Block = 177,                    /* Block  */
  YYSYMBOL_BlockStatementsopt = 178,       /* BlockStatementsopt  */
  YYSYMBOL_BlockStatements = 179,          /* BlockStatements  */
  YYSYMBOL_BlockStatement = 180,           /* BlockStatement  */
  YYSYMBOL_LocalVariableDeclarationStatement = 181, /* LocalVariableDeclarationStatement  */
  YYSYMBOL_LocalVariableDeclaration = 182, /* LocalVariableDeclaration  */
  YYSYMBOL_Statement = 183,                /* Statement  */
  YYSYMBOL_StatementNoShortIf = 184,       /* StatementNoShortIf  */
  YYSYMBOL_StatementWithoutTrailingSubstatement = 185, /* StatementWithoutTrailingSubstatement  */
  YYSYMBOL_EmptyStatement = 186,           /* EmptyStatement  */
  YYSYMBOL_LabeledStatement = 187,         /* LabeledStatement  */
  YYSYMBOL_LabeledStatementNoShortIf = 188, /* LabeledStatementNoShortIf  */
  YYSYMBOL_ExpressionStatement = 189,      /* ExpressionStatement  */
  YYSYMBOL_StatementExpression = 190,      /* StatementExpression  */
  YYSYMBOL_IfThenStatement = 191,          /* IfThenStatement  */
  YYSYMBOL_IfThenElseStatement = 192,      /* IfThenElseStatement  */
  YYSYMBOL_IfThenElseStatementNoShortIf = 193, /* IfThenElseStatementNoShortIf  */
  YYSYMBOL_SwitchStatement = 194,          /* SwitchStatement  */
  YYSYMBOL_SwitchBlock = 195,              /* SwitchBlock  */
  YYSYMBOL_SwitchLabelsopt = 196,          /* SwitchLabelsopt  */
  YYSYMBOL_SwitchBlockStatementGroups = 197, /* SwitchBlockStatementGroups  */
  YYSYMBOL_SwitchBlockStatementGroup = 198, /* SwitchBlockStatementGroup  */
  YYSYMBOL_SwitchLabels = 199,             /* SwitchLabels  */
  YYSYMBOL_SwitchLabel = 200,              /* SwitchLabel  */
  YYSYMBOL_WhileStatement = 201,           /* WhileStatement  */
  YYSYMBOL_WhileStatementNoShortIf = 202,  /* WhileStatementNoShortIf  */
  YYSYMBOL_DoStatement = 203,              /* DoStatement  */
  YYSYMBOL_ForStatement = 204,             /* ForStatement  */
  YYSYMBOL_ForUpdateopt = 205,             /* ForUpdateopt  */
  YYSYMBOL_ForInitopt = 206,               /* ForInitopt  */
  YYSYMBOL_ForStatementNoShortIf = 207,    /* ForStatementNoShortIf  */
  YYSYMBOL_Expressionopt = 208,            /* Expressionopt  */
  YYSYMBOL_ForInit = 209,                  /* ForInit  */
  YYSYMBOL_ForUpdate = 210,                /* ForUpdate  */
  YYSYMBOL_StatementExpressionList = 211,  /* StatementExpressionList  */
  YYSYMBOL_AssertStatement = 212,          /* AssertStatement  */
  YYSYMBOL_BreakStatement = 213,           /* BreakStatement  */
  YYSYMBOL_Identifieropt = 214,            /* Identifieropt  */
  YYSYMBOL_ContinueStatement = 215,        /* ContinueStatement  */
  YYSYMBOL_ReturnStatement = 216,          /* ReturnStatement  */
  YYSYMBOL_ThrowStatement = 217,           /* ThrowStatement  */
  YYSYMBOL_SynchronizedStatement = 218,    /* SynchronizedStatement  */
  YYSYMBOL_TryStatement = 219,             /* TryStatement  */
  YYSYMBOL_Catchesopt = 220,               /* Catchesopt  */
  YYSYMBOL_Catches = 221,                  /* Catches  */
  YYSYMBOL_CatchClause = 222,              /* CatchClause  */
  YYSYMBOL_Finally = 223,                  /* Finally  */
  YYSYMBOL_Primary = 224,                  /* Primary  */
  YYSYMBOL_PrimaryNoNewArray = 225,        /* PrimaryNoNewArray  */
  YYSYMBOL_ClassInstanceCreationExpression = 226, /* ClassInstanceCreationExpression  */
  YYSYMBOL_ClassBodyOpt = 227,             /* ClassBodyOpt  */
  YYSYMBOL_ArgumentListopt = 228,          /* ArgumentListopt  */
  YYSYMBOL_ArgumentList = 229,             /* ArgumentList  */
  YYSYMBOL_ArrayCreationExpression = 230,  /* ArrayCreationExpression  */
  YYSYMBOL_Dimsopt = 231,                  /* Dimsopt  */
  YYSYMBOL_DimExprs = 232,                 /* DimExprs  */
  YYSYMBOL_DimExpr = 233,                  /* DimExpr  */
  YYSYMBOL_Dims = 234,                     /* Dims  */
  YYSYMBOL_FieldAccess = 235,              /* FieldAccess  */
  YYSYMBOL_MethodInvocation = 236,         /* MethodInvocation  */
  YYSYMBOL_ArrayAccess = 237,              /* ArrayAccess  */
  YYSYMBOL_PostfixExpression = 238,        /* PostfixExpression  */
  YYSYMBOL_PostIncrementExpression = 239,  /* PostIncrementExpression  */
  YYSYMBOL_PostDecrementExpression = 240,  /* PostDecrementExpression  */
  YYSYMBOL_UnaryExpression = 241,          /* UnaryExpression  */
  YYSYMBOL_PreIncrementExpression = 242,   /* PreIncrementExpression  */
  YYSYMBOL_PreDecrementExpression = 243,   /* PreDecrementExpression  */
  YYSYMBOL_UnaryExpressionNotPlusMinus = 244, /* UnaryExpressionNotPlusMinus  */
  YYSYMBOL_CastExpression = 245,           /* CastExpression  */
  YYSYMBOL_MultiplicativeExpression = 246, /* MultiplicativeExpression  */
  YYSYMBOL_AdditiveExpression = 247,       /* AdditiveExpression  */
  YYSYMBOL_ShiftExpression = 248,          /* ShiftExpression  */
  YYSYMBOL_RelationalExpression = 249,     /* RelationalExpression  */
  YYSYMBOL_EqualityExpression = 250,       /* EqualityExpression  */
  YYSYMBOL_AndExpression = 251,            /* AndExpression  */
  YYSYMBOL_ExclusiveOrExpression = 252,    /* ExclusiveOrExpression  */
  YYSYMBOL_InclusiveOrExpression = 253,    /* InclusiveOrExpression  */
  YYSYMBOL_ConditionalAndExpression = 254, /* ConditionalAndExpression  */
  YYSYMBOL_ConditionalOrExpression = 255,  /* ConditionalOrExpression  */
  YYSYMBOL_ConditionalExpression = 256,    /* ConditionalExpression  */
  YYSYMBOL_AssignmentExpression = 257,     /* AssignmentExpression  */
  YYSYMBOL_Assignment = 258,               /* Assignment  */
  YYSYMBOL_LeftHandSide = 259,             /* LeftHandSide  */
  YYSYMBOL_AssignmentOperator = 260,       /* AssignmentOperator  */
  YYSYMBOL_Expression = 261,               /* Expression  */
  YYSYMBOL_ConstantExpression = 262,       /* ConstantExpression  */
  YYSYMBOL_New = 263                       /* New  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  23
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2215

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  106
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  158
/* YYNRULES -- Number of rules.  */
#define YYNRULES  351
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  575

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   360


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   184,   184,   193,   201,   209,   217,   225,   233,   242,
     250,   259,   267,   276,   281,   286,   291,   296,   301,   306,
     311,   317,   325,   334,   344,   353,   362,   370,   380,   386,
     393,   400,   406,   413,   422,   432,   442,   451,   459,   468,
     477,   483,   492,   498,   507,   513,   522,   534,   542,   551,
     563,   576,   584,   592,   601,   609,   618,   618,   618,   619,
     620,   620,   620,   620,   620,   620,   621,   624,   634,   643,
     652,   661,   671,   677,   686,   695,   704,   712,   721,   730,
     736,   745,   753,   761,   769,   778,   786,   795,   801,   809,
     818,   826,   835,   844,   853,   861,   870,   878,   886,   895,
     904,   914,   921,   931,   941,   948,   955,   958,   964,   974,
     984,   994,  1000,  1010,  1020,  1030,  1039,  1049,  1060,  1070,
    1077,  1087,  1096,  1106,  1115,  1125,  1131,  1141,  1150,  1160,
    1170,  1177,  1186,  1195,  1204,  1213,  1221,  1230,  1239,  1249,
    1259,  1268,  1278,  1288,  1295,  1304,  1314,  1323,  1333,  1342,
    1349,  1359,  1368,  1378,  1387,  1396,  1406,  1416,  1425,  1435,
    1444,  1453,  1462,  1471,  1480,  1490,  1499,  1508,  1517,  1526,
    1536,  1545,  1554,  1563,  1572,  1581,  1590,  1599,  1608,  1617,
    1626,  1635,  1645,  1655,  1666,  1676,  1686,  1695,  1704,  1713,
    1722,  1731,  1740,  1750,  1760,  1770,  1780,  1787,  1794,  1801,
    1811,  1818,  1828,  1838,  1847,  1857,  1866,  1876,  1883,  1890,
    1897,  1905,  1912,  1922,  1929,  1939,  1949,  1956,  1966,  1975,
    1985,  1995,  2004,  2014,  2023,  2033,  2044,  2051,  2058,  2069,
    2079,  2089,  2099,  2108,  2118,  2125,  2135,  2144,  2154,  2161,
    2171,  2180,  2190,  2199,  2205,  2214,  2223,  2232,  2241,  2251,
    2261,  2268,  2278,  2285,  2295,  2304,  2314,  2323,  2332,  2341,
    2351,  2358,  2368,  2377,  2387,  2397,  2403,  2410,  2420,  2430,
    2440,  2451,  2461,  2472,  2482,  2493,  2503,  2513,  2522,  2531,
    2540,  2549,  2559,  2569,  2579,  2588,  2597,  2606,  2615,  2625,
    2635,  2645,  2654,  2663,  2672,  2682,  2691,  2700,  2707,  2716,
    2725,  2734,  2744,  2753,  2762,  2772,  2781,  2790,  2799,  2809,
    2818,  2827,  2836,  2845,  2854,  2864,  2873,  2882,  2892,  2901,
    2911,  2920,  2930,  2939,  2949,  2958,  2968,  2977,  2987,  2996,
    3006,  3015,  3025,  3035,  3045,  3054,  3064,  3073,  3082,  3091,
    3100,  3109,  3118,  3127,  3136,  3145,  3154,  3163,  3173,  3183,
    3193,  3202
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "jp_ABSTRACT",
  "jp_ASSERT", "jp_BOOLEAN_TYPE", "jp_BREAK", "jp_BYTE_TYPE", "jp_CASE",
  "jp_CATCH", "jp_CHAR_TYPE", "jp_CLASS", "jp_CONTINUE", "jp_DEFAULT",
  "jp_DO", "jp_DOUBLE_TYPE", "jp_ELSE", "jp_EXTENDS", "jp_FINAL",
  "jp_FINALLY", "jp_FLOAT_TYPE", "jp_FOR", "jp_IF", "jp_IMPLEMENTS",
  "jp_IMPORT", "jp_INSTANCEOF", "jp_INT_TYPE", "jp_INTERFACE",
  "jp_LONG_TYPE", "jp_NATIVE", "jp_NEW", "jp_PACKAGE", "jp_PRIVATE",
  "jp_PROTECTED", "jp_PUBLIC", "jp_RETURN", "jp_SHORT_TYPE", "jp_STATIC",
  "jp_STRICTFP", "jp_SUPER", "jp_SWITCH", "jp_SYNCHRONIZED", "jp_THIS",
  "jp_THROW", "jp_THROWS", "jp_TRANSIENT", "jp_TRY", "jp_VOID",
  "jp_VOLATILE", "jp_WHILE", "jp_BOOLEANLITERAL", "jp_CHARACTERLITERAL",
  "jp_DECIMALINTEGERLITERAL", "jp_FLOATINGPOINTLITERAL",
  "jp_HEXINTEGERLITERAL", "jp_NULLLITERAL", "jp_STRINGLITERAL", "jp_NAME",
  "jp_AND", "jp_ANDAND", "jp_ANDEQUALS", "jp_BRACKETEND",
  "jp_BRACKETSTART", "jp_CARROT", "jp_CARROTEQUALS", "jp_COLON",
  "jp_COMMA", "jp_CURLYEND", "jp_CURLYSTART", "jp_DIVIDE",
  "jp_DIVIDEEQUALS", "jp_DOLLAR", "jp_DOT", "jp_EQUALS", "jp_EQUALSEQUALS",
  "jp_EXCLAMATION", "jp_EXCLAMATIONEQUALS", "jp_GREATER", "jp_GTEQUALS",
  "jp_GTGT", "jp_GTGTEQUALS", "jp_GTGTGT", "jp_GTGTGTEQUALS",
  "jp_LESLESEQUALS", "jp_LESSTHAN", "jp_LTEQUALS", "jp_LTLT", "jp_MINUS",
  "jp_MINUSEQUALS", "jp_MINUSMINUS", "jp_PAREEND", "jp_PARESTART",
  "jp_PERCENT", "jp_PERCENTEQUALS", "jp_PIPE", "jp_PIPEEQUALS",
  "jp_PIPEPIPE", "jp_PLUS", "jp_PLUSEQUALS", "jp_PLUSPLUS", "jp_QUESTION",
  "jp_SEMICOL", "jp_TILDE", "jp_TIMES", "jp_TIMESEQUALS", "jp_ERROR",
  "$accept", "Goal", "Literal", "IntegerLiteral", "Type", "PrimitiveType",
  "ReferenceType", "ClassOrInterfaceType", "ClassType", "InterfaceType",
  "ArrayType", "Name", "SimpleName", "Identifier", "QualifiedName",
  "SimpleType", "CompilationUnit", "PackageDeclarationopt",
  "ImportDeclarations", "TypeDeclarations", "PackageDeclaration",
  "ImportDeclaration", "SingleTypeImportDeclaration",
  "TypeImportOnDemandDeclaration", "TypeDeclaration", "Modifiers",
  "Modifier", "ClassHeader", "ClassDeclaration", "Modifiersopt", "Super",
  "Interfaces", "InterfaceTypeList", "ClassBody", "ClassBodyDeclarations",
  "ClassBodyDeclaration", "ClassMemberDeclaration", "FieldDeclaration",
  "VariableDeclarators", "VariableDeclarator", "VariableDeclaratorId",
  "VariableInitializer", "MethodDeclaration", "MethodHeader", "Throwsopt",
  "MethodDeclarator", "FormalParameterListopt", "FormalParameterList",
  "FormalParameter", "Throws", "ClassTypeList", "MethodBody",
  "StaticInitializer", "ConstructorDeclaration", "ConstructorDeclarator",
  "ConstructorBody", "ExplicitConstructorInvocationopt",
  "ExplicitConstructorInvocation", "InterfaceHeader",
  "InterfaceDeclaration", "ExtendsInterfacesopt", "ExtendsInterfaces",
  "InterfaceBody", "InterfaceMemberDeclarations",
  "InterfaceMemberDeclaration", "ConstantDeclaration",
  "AbstractMethodDeclaration", "Semicols", "ArrayInitializer",
  "VariableInitializersOptional", "VariableInitializers", "Block",
  "BlockStatementsopt", "BlockStatements", "BlockStatement",
  "LocalVariableDeclarationStatement", "LocalVariableDeclaration",
  "Statement", "StatementNoShortIf",
  "StatementWithoutTrailingSubstatement", "EmptyStatement",
  "LabeledStatement", "LabeledStatementNoShortIf", "ExpressionStatement",
  "StatementExpression", "IfThenStatement", "IfThenElseStatement",
  "IfThenElseStatementNoShortIf", "SwitchStatement", "SwitchBlock",
  "SwitchLabelsopt", "SwitchBlockStatementGroups",
  "SwitchBlockStatementGroup", "SwitchLabels", "SwitchLabel",
  "WhileStatement", "WhileStatementNoShortIf", "DoStatement",
  "ForStatement", "ForUpdateopt", "ForInitopt", "ForStatementNoShortIf",
  "Expressionopt", "ForInit", "ForUpdate", "StatementExpressionList",
  "AssertStatement", "BreakStatement", "Identifieropt",
  "ContinueStatement", "ReturnStatement", "ThrowStatement",
  "SynchronizedStatement", "TryStatement", "Catchesopt", "Catches",
  "CatchClause", "Finally", "Primary", "PrimaryNoNewArray",
  "ClassInstanceCreationExpression", "ClassBodyOpt", "ArgumentListopt",
  "ArgumentList", "ArrayCreationExpression", "Dimsopt", "DimExprs",
  "DimExpr", "Dims", "FieldAccess", "MethodInvocation", "ArrayAccess",
  "PostfixExpression", "PostIncrementExpression",
  "PostDecrementExpression", "UnaryExpression", "PreIncrementExpression",
  "PreDecrementExpression", "UnaryExpressionNotPlusMinus",
  "CastExpression", "MultiplicativeExpression", "AdditiveExpression",
  "ShiftExpression", "RelationalExpression", "EqualityExpression",
  "AndExpression", "ExclusiveOrExpression", "InclusiveOrExpression",
  "ConditionalAndExpression", "ConditionalOrExpression",
  "ConditionalExpression", "AssignmentExpression", "Assignment",
  "LeftHandSide", "AssignmentOperator", "Expression", "ConstantExpression",
  "New", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-503)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-336)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     159,  1039,   236,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,   186,  -503,    56,  -503,
    -503,  -503,   178,  -503,    35,  -503,    21,  -503,   248,  1039,
     273,  -503,  -503,  -503,  -503,  -503,  -503,  -503,    78,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  2088,  -503,    32,  -503,    16,   245,  -503,    28,
    -503,  -503,  1039,  1039,  -503,    80,   206,  -503,   129,   129,
    1039,   221,   228,   194,  -503,  -503,   225,  -503,  -503,   234,
     164,   206,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  1039,
    -503,  1039,   233,  -503,  -503,   739,  -503,  -503,  -503,  -503,
     -49,  -503,  -503,  -503,  1116,  -503,  -503,  1276,  -503,   129,
     129,    40,  -503,  -503,  -503,   122,   212,   265,  -503,   215,
    -503,  -503,   219,   739,  -503,   222,   224,  -503,  -503,  -503,
    1820,   129,   129,  1627,   237,   238,  -503,  1820,   241,   239,
     242,   283,  1820,   233,   266,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  1820,  1820,  1820,  -503,  -503,  -503,   129,   284,
     476,   293,  2067,  -503,   349,  -503,   296,  1366,  -503,  -503,
     264,  -503,  -503,  -503,  -503,  -503,   268,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
     294,   305,    72,  -503,  2070,    88,  2084,   121,   130,   148,
    -503,  -503,  -503,  2111,  1039,   281,   133,   281,   -25,  -503,
     126,   133,   314,   315,   315,   921,  1039,   308,  -503,  -503,
    -503,  -503,   277,  -503,  1820,  1820,  1820,  1820,  1820,   317,
     284,   545,  -503,  -503,   121,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,    73,   124,   163,    59,   196,   323,   319,   290,
     324,    18,  -503,  -503,  -503,   -30,  -503,   285,   286,   242,
     342,  1941,  1820,   291,  -503,   129,  1820,  1820,   129,   292,
     385,  1820,    96,  -503,  -503,  -503,   310,  -503,  -503,   329,
     387,  1085,     3,  1820,  1627,   129,  -503,  -503,  -503,  -503,
     175,  1820,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  1820,   339,   339,   311,
     921,   343,  -503,   129,  -503,   344,  1766,  -503,  -503,   346,
    1039,   313,   347,  -503,  -503,   353,  -503,   307,  -503,  -503,
    -503,     6,   545,   326,  -503,  -503,  1820,  1820,  1820,  1820,
    1820,  1820,  1820,  1820,  1039,  1820,  1820,  1820,  1820,  1820,
    1820,  1820,  1820,  1820,  1820,  1820,  1820,  1820,  -503,  -503,
    -503,   330,  2067,  -503,  -503,   327,  -503,   354,   334,  -503,
     345,   335,   340,   348,  -503,   351,   416,   232,  -503,   356,
    -503,  -503,   376,  -503,   357,   377,  -503,  -503,   329,  -503,
     358,   390,  -503,  1085,   339,  -503,   154,   339,   154,  1820,
     362,  -503,  -503,  -503,  1766,  -503,  -503,  -503,  -503,   129,
    -503,  2088,  1039,  1456,  -503,   363,    70,    93,  1874,  -503,
    -503,  -503,    73,    73,   124,   124,   124,  -503,   163,   163,
     163,   163,    59,    59,   196,   323,   319,   290,   324,   383,
     360,  1820,  1820,  1995,  1699,  1820,   386,   233,  1820,  2088,
     233,  -503,  -503,  1627,  -503,  -503,  1820,  1820,  -503,   394,
    -503,  -503,   315,  -503,  -503,  -503,   369,  -503,  -503,   396,
     404,   410,  -503,  -503,    26,   113,  -503,   407,  1820,  1874,
    -503,  1820,  -503,   391,   374,  -503,   393,   395,   397,   411,
    -503,   466,   471,  -503,  -503,  -503,  -503,   399,  -503,  -503,
    -503,   400,   401,  -503,  -503,  -503,   402,  -503,   206,  -503,
    1766,  1820,  1820,  -503,  -503,  -503,  -503,   403,  1995,  1941,
    1820,  1820,  1699,  1627,  -503,    34,  -503,   233,  -503,  -503,
    -503,  -503,   405,   412,  -503,   413,  -503,   354,   406,   418,
     421,  -503,  -503,  1820,   429,   430,  -503,  1186,  -503,  -503,
     419,   422,  1627,  1820,  1699,  1699,  -503,   447,  -503,  -503,
    1555,  -503,  -503,  -503,  -503,   423,   497,  -503,  -503,  1995,
    1699,   432,  -503,  1699,  -503
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
      40,     0,     0,     2,    42,    41,    20,    13,    17,    19,
      18,    15,    16,    14,    38,    31,     0,    37,     0,    28,
      30,    29,     0,     1,    44,    32,     0,    46,     0,     0,
      72,    43,    47,    48,    34,    35,    33,    36,     0,    60,
      61,    62,    58,    57,    56,    59,    66,    63,    64,    65,
      53,    45,    73,    54,     0,    51,     0,   125,    52,     0,
      49,    55,     0,     0,    79,     0,     0,    68,     0,     0,
       0,     0,   126,     0,    24,    74,    23,    25,    76,    75,
      72,     0,    70,    69,    67,   123,   127,   130,   124,     0,
      50,     0,    59,    78,    84,     0,    80,    81,    85,    86,
       0,    82,    83,    71,    72,   128,    77,    72,   114,    38,
       0,    11,    12,    21,    22,    23,    28,   101,    96,    97,
     113,   129,   134,     0,   138,     0,   136,   131,   132,   133,
       0,   226,   226,     0,     0,     0,   350,   216,     0,     0,
      63,   243,     0,     0,     0,     5,     6,     9,     4,    10,
       8,     7,     0,     0,     0,   182,   242,     3,     0,    22,
     333,    30,    73,   155,     0,   170,     0,    72,   151,   153,
       0,   154,   159,   171,   160,   172,     0,   161,   162,   173,
     163,   174,   164,   181,   175,   176,   177,   179,   178,   180,
     277,   240,   245,   241,   246,   247,   248,     0,   189,   190,
     187,   188,   186,     0,     0,     0,   101,    92,     0,    88,
      90,   101,     0,    26,    27,    72,     0,     0,   102,    98,
     135,   140,   139,   137,     0,     0,     0,     0,     0,    37,
       0,   278,   245,   247,   291,   280,   281,   298,   284,   285,
     288,   294,   302,   305,   309,   315,   318,   320,   322,   324,
     326,   328,   330,   348,   331,     0,   227,     0,     0,     0,
       0,   213,     0,     0,   217,     0,     0,     0,     0,     0,
     234,     0,   278,   246,   248,   290,     0,   289,    92,   158,
       0,     0,     0,   252,     0,     0,   148,   152,   156,   185,
       0,     0,   283,   282,   345,   346,   338,   336,   343,   344,
     342,   341,   339,   347,   340,   337,     0,    37,    24,     0,
      72,     0,   100,     0,    87,     0,     0,    99,   265,     0,
       0,     0,   106,   107,   111,   110,   119,   115,   141,   293,
     287,    37,   278,     0,   286,   292,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   223,   225,
     228,     0,     0,   219,   221,     0,   214,   218,     0,   229,
     268,     0,     0,   269,   230,     0,     0,   232,   236,     0,
     244,   279,     0,   351,     0,   253,   254,   183,   157,   270,
     267,     0,   332,     0,   260,   262,     0,   260,     0,   252,
       0,   104,    89,    93,   143,    91,    95,    94,   266,     0,
     117,    72,     0,    72,   116,     0,    26,    27,   244,   300,
     301,   299,   304,   303,   307,   308,   306,   314,   311,   313,
     310,   312,   316,   317,   319,   321,   323,   325,   327,     0,
       0,     0,   216,     0,     0,   252,     0,     0,   252,    72,
       0,   233,   237,     0,   275,   271,     0,   252,   276,     0,
     256,   263,   261,   258,   257,   259,     0,   103,   146,     0,
     144,   109,   108,   112,     0,   243,   120,     0,     0,     0,
     296,     0,   224,     0,     0,   222,     0,     0,     0,    30,
     193,     0,   159,   166,   167,   168,   169,     0,   200,   196,
     231,     0,     0,   239,   207,   255,     0,   264,   250,   142,
     145,   252,   252,   118,   295,   297,   329,     0,   211,   213,
       0,     0,     0,     0,   273,   198,   274,     0,   272,   251,
     249,   147,     0,     0,   209,     0,   212,   220,     0,     0,
       0,   184,   194,     0,     0,     0,   201,    72,   203,   238,
       0,     0,     0,   216,     0,     0,   349,     0,   206,   197,
     202,   204,   122,   121,   210,     0,     0,   208,   205,   211,
       0,     0,   195,     0,   215
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -503,  -503,  -503,  -503,   -85,     2,   181,   -41,  -198,   -45,
     -87,    -1,   431,    14,  -503,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,   448,   -81,   -47,  -503,     7,   -23,
    -503,   462,  -503,   -64,  -503,  -503,  -503,   425,  -146,   217,
     123,  -391,  -503,   427,  -101,   424,   230,  -503,  -360,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,   439,
    -503,  -503,  -503,  -503,  -503,  -503,  -503,  -503,  -110,  -503,
    -503,   -77,   138,   -12,  -163,  -503,  -250,   -13,  -421,  -414,
    -503,  -503,  -503,  -503,  -252,  -503,  -503,  -503,  -503,  -503,
    -503,  -503,  -503,  -503,     5,  -503,  -503,  -503,  -503,   -16,
      36,  -503,  -418,  -503,  -503,  -502,  -503,  -503,   440,  -503,
    -503,  -503,  -503,  -503,  -503,  -503,   179,  -503,  -503,  -503,
     -54,  -503,  -341,  -503,  -503,  -149,   255,  -136,   102,   652,
     101,   688,   145,   157,   201,   -98,   289,   338,  -384,  -503,
     -59,   -58,   -92,   -57,   213,   226,   218,   223,   227,  -503,
      95,   274,   350,  -503,  -503,   660,  -503,  -503
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,   156,   157,   158,   229,   112,   113,    75,    78,
     230,   231,    19,    20,    21,    22,     3,     4,    24,    30,
       5,    31,    32,    33,    51,    52,    53,    54,   163,   164,
      65,    66,    79,    67,    80,    96,    97,    98,   208,   209,
     210,   405,    99,   100,   217,   206,   321,   322,   323,   218,
     325,   119,   101,   102,   117,   327,   413,   476,    57,    58,
      71,    72,    88,   104,   127,   128,   129,   222,   406,   469,
     470,   165,   166,   167,   168,   169,   170,   171,   491,   172,
     173,   174,   493,   175,   176,   177,   178,   494,   179,   499,
     545,   525,   546,   547,   548,   180,   495,   181,   182,   535,
     365,   496,   263,   366,   536,   367,   183,   184,   257,   185,
     186,   187,   188,   189,   376,   377,   378,   451,   190,   191,
     232,   530,   384,   385,   193,   415,   394,   395,   214,   194,
     233,   196,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   203,   306,   386,   557,   204
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      18,    82,    83,    17,   287,    61,   309,    56,   114,   364,
     110,   363,   279,   468,    34,   108,   537,   103,   324,   107,
     159,    74,    77,   120,   484,    86,   162,    68,    38,    77,
     492,    17,    34,   383,   480,   357,   114,    55,   110,    34,
      36,   313,   543,    69,   105,    35,   106,   544,    77,    62,
      77,   472,   118,   192,   275,    63,   277,    95,   466,    29,
      15,    76,    76,    35,    17,    17,   270,   537,   212,    76,
      35,   358,    17,    36,    16,   114,   314,   285,    15,   192,
     159,   123,    84,    85,   344,    15,   162,    55,    76,   502,
      76,    17,    16,    17,   115,   515,  -260,   111,   265,    16,
      64,   541,   212,    63,   497,   312,   160,   501,   492,   111,
     317,   122,   -37,   192,   355,    61,   506,   511,   356,   531,
     260,   161,   115,   205,   207,   111,   329,   330,    26,   334,
     335,    73,   319,   566,   567,   565,   345,   346,  -192,   388,
     492,   492,   336,   347,   348,   256,   256,   161,    64,   572,
      59,   272,   574,   272,  -191,   319,   492,    27,   281,   492,
    -261,   115,  -192,   308,   111,   337,   160,    39,   282,   111,
     532,   533,   278,  -192,   159,    74,   338,   216,  -191,    60,
     362,   161,    40,   479,   212,   268,    15,   283,   315,  -191,
       1,   485,   320,    41,    26,   311,    42,    43,    44,   316,
      16,    92,    46,    76,   512,    47,   307,   192,   195,    48,
     292,   339,    49,   213,   473,    76,   319,   389,    17,  -280,
     293,   340,   404,   272,   272,   332,   272,   272,   331,  -280,
     192,    93,    15,   114,   195,   409,    23,  -281,   419,   420,
     421,   375,   341,    25,   342,   460,    16,  -281,   464,   343,
      28,  -235,   197,   428,   429,   430,   431,   114,   461,    37,
     160,   461,    70,   111,   198,    50,   364,   364,   195,   363,
     349,   387,   350,   -39,    64,   114,    39,   285,   197,   370,
     422,   423,   373,   424,   425,   426,   463,   320,   465,    87,
     198,    40,   432,   433,    89,    90,    36,    26,   161,   278,
      91,   107,    41,   215,   390,    42,    43,    44,   199,   216,
      45,    46,   197,   265,    47,    61,   219,   364,    48,   115,
     220,    49,   111,   221,   198,   223,   159,   278,   261,   262,
     266,   213,   162,   267,   199,   272,   272,   272,   272,   272,
     272,   272,   272,   115,   272,   272,   272,   272,   272,   272,
     272,   272,   272,   272,   272,   268,   280,   271,   284,   192,
      68,   115,   195,   286,   111,   288,   290,   291,   199,   289,
     500,    74,   310,   503,    50,   318,   326,   319,   328,   212,
     514,   351,   352,   354,   353,   195,   359,   360,   320,   192,
     192,   361,   369,   374,   375,   313,   200,   287,   381,   192,
     380,   393,   399,   410,   401,   403,   197,   408,   414,   396,
     398,    76,   160,   411,    17,   111,   418,   272,   198,   412,
     443,   441,   200,   278,   444,   446,   320,   161,   442,   197,
     447,   490,   159,   416,   417,   450,   445,   454,   362,   448,
     504,   198,   449,   456,   529,   201,   453,   455,   481,   457,
     549,   458,   467,   478,   498,   507,   200,   202,   489,   508,
     159,   482,   199,   509,   192,   192,   162,   161,   192,   192,
     510,   201,   315,   159,   513,   518,   522,   272,   272,   162,
     272,   517,   523,   202,   519,   199,   520,  -165,   521,   524,
     526,   527,   528,   192,   558,   550,   462,   559,   192,   462,
     192,   192,   551,   552,   534,   201,   192,   553,   554,   387,
     542,   555,   568,   570,   195,   192,   192,   202,   160,   192,
     562,   111,   573,   563,   569,   427,   116,    81,    94,   124,
     402,   125,   471,   -23,   211,   560,   489,   161,   281,   564,
     400,   490,   504,   126,   195,   195,   160,   -23,   282,   111,
     200,   477,   561,   571,   195,   538,   452,   542,   197,   160,
     564,   161,   111,   397,   434,  -278,   161,   283,   489,   489,
     198,   436,   258,   200,   161,  -278,   516,   437,   435,     0,
     392,     0,   438,     0,   489,     0,     0,   489,   197,   197,
       0,     0,     0,     0,     0,     0,     0,     0,   197,   201,
     198,   198,     0,     0,     0,  -333,     0,   281,     0,  -333,
     198,   202,     0,     0,   199,  -333,     0,   282,  -333,   195,
     195,     0,   201,   195,   195,  -333,     0,  -333,  -333,     0,
       0,     0,     0,  -333,   202,     0,   283,     0,  -333,     0,
    -333,     0,     0,  -333,   199,   199,     0,     0,   195,  -333,
       0,     0,     0,   195,   199,   195,   195,     0,     0,     0,
       0,   195,     0,   197,   197,     0,     0,   197,   197,     0,
     195,   195,     0,     0,   195,   198,   198,     0,     0,   198,
     198,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   197,     0,     0,     0,     0,   197,     0,   197,
     197,     0,   200,     0,   198,   197,     0,     0,     0,   198,
       0,   198,   198,     0,   197,   197,     0,   198,   197,   199,
     199,     0,     0,   199,   199,     0,   198,   198,     0,     0,
     198,     0,   200,   200,     0,     0,     0,     0,     0,     0,
       0,     0,   200,     0,     6,     0,     7,     0,   199,     8,
      68,   201,     0,   199,     9,   199,   199,     0,     0,    10,
       0,   199,     0,   202,     0,    11,    69,    12,     0,     0,
     199,   199,     0,     0,   199,    13,     0,     0,     0,     0,
       0,   201,   201,     0,     0,     0,   109,     0,     0,     0,
     255,   201,     0,   202,   202,     0,    15,   264,     0,     0,
       0,     0,   269,   202,   273,     0,   273,   200,   200,     0,
      16,   200,   200,   276,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   200,     0,     0,     0,
     274,   200,   274,   200,   200,     0,     0,     0,     0,   200,
       0,     0,     0,     0,     0,     0,   201,   201,   200,   200,
     201,   201,   200,     0,     0,     0,     0,     0,   202,   202,
       0,     0,   202,   202,     0,     0,   273,   273,     0,   273,
     273,     0,     0,     0,     0,   201,   333,     0,     0,     0,
     201,     0,   201,   201,     0,     0,     0,   202,   201,     0,
       0,     0,   202,     0,   202,   202,     0,   201,   201,     0,
     202,   201,   274,   274,     0,   274,   274,     0,     0,   202,
     202,     0,   368,   202,    39,     0,   371,   372,     0,     0,
       0,   379,     0,     0,     0,     0,     0,     0,     0,    40,
       0,   382,     0,     0,     0,     0,     0,     0,     0,     0,
      41,   391,     0,    42,    43,    44,     0,     0,    45,    46,
       0,     0,    47,     0,     0,     0,    48,     0,     0,    49,
       0,     0,     0,     0,     0,     0,   407,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   273,   273,
     273,   273,   273,   273,   273,   273,     0,   273,   273,   273,
     273,   273,   273,   273,   273,   273,   273,   273,     0,     0,
       0,  -105,     0,     0,     0,     0,   439,   440,     0,     0,
       0,     0,     0,     0,   274,   274,   274,   274,   274,   274,
     274,   274,     0,   274,   274,   274,   274,   274,   274,   274,
     274,   274,   274,   274,     6,     0,     7,     0,     0,     8,
       0,     0,     0,   459,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,   407,    11,     0,    12,     0,     0,
     273,     0,     0,     0,     0,    13,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    14,     0,     0,     0,
       6,     0,     7,     0,     0,     8,    15,     0,     0,     0,
       9,   483,   264,     0,     0,    10,   274,     0,     0,     0,
      16,    11,     0,    12,     0,   136,   505,     0,     0,    39,
       0,    13,     0,     0,   138,     0,     0,   141,     0,     0,
     273,   273,    14,   273,    40,   145,   146,   147,   148,   149,
     150,   151,    15,     0,     0,    41,   318,     0,    42,    43,
      44,     0,     0,    45,    46,     0,    16,    47,     0,     0,
     224,    48,     0,     0,    49,     0,   274,   274,     0,   274,
     407,     0,   225,     0,   152,     0,   226,     0,     0,     0,
     539,   540,   227,   121,   154,     0,     0,   228,     0,    39,
     130,     6,   131,     7,   543,     0,     8,     0,   132,   544,
     133,     9,     0,   556,    40,     0,    10,   134,   135,     0,
       0,     0,    11,   264,    12,    41,   136,     0,    42,    43,
      44,   137,    13,    45,    46,   138,   139,   140,   141,   142,
       0,    48,   143,    14,    49,   144,   145,   146,   147,   148,
     149,   150,   151,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -199,   107,     0,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,     0,   153,     0,    39,
     130,     6,   131,     7,     0,   154,     8,   155,   132,     0,
     133,     9,     0,     0,    40,     0,    10,   134,   135,     0,
       0,     0,    11,     0,    12,    41,   136,     0,    42,    43,
      44,   137,    13,    45,    46,   138,   139,   140,   141,   142,
       0,    48,   143,    14,    49,   144,   145,   146,   147,   148,
     149,   150,   151,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -149,   107,     0,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,     0,   153,     0,    39,
     130,     6,   131,     7,     0,   154,     8,   155,   132,     0,
     133,     9,     0,     0,    40,     0,    10,   134,   135,     0,
       0,     0,    11,     0,    12,    41,   136,     0,    42,    43,
      44,   137,    13,    45,    46,   138,   139,   140,   141,   142,
       0,    48,   143,    14,    49,   144,   145,   146,   147,   148,
     149,   150,   151,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -150,   107,     0,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,     0,   153,     0,    39,
     130,     6,   131,     7,     0,   154,     8,   155,   132,     0,
     133,     9,     0,     0,    40,     0,    10,   134,   135,     0,
       0,     0,    11,     0,    12,    41,   136,     0,    42,    43,
      44,   137,    13,    45,    46,   474,   139,   140,   475,   142,
       0,    48,   143,    14,    49,   144,   145,   146,   147,   148,
     149,   150,   151,    15,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  -149,   107,     0,     0,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   152,     0,   153,     0,     0,
       0,     0,     0,     0,     0,   154,     0,   155,    39,   130,
       6,   131,     7,     0,     0,     8,   -72,   132,     0,   133,
       9,     0,     0,    40,     0,    10,   134,   135,     0,     0,
       0,    11,     0,    12,    41,   136,     0,    42,    43,    44,
     137,    13,    45,    46,   138,   139,   140,   141,   142,     0,
      48,   143,    14,    49,   144,   145,   146,   147,   148,   149,
     150,   151,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   107,     0,     0,    16,     0,     0,     0,
       0,   130,     6,   131,     7,     0,     0,     8,     0,   132,
       0,   133,     9,     0,   152,     0,   153,    10,   134,   135,
       0,     0,     0,    11,   154,    12,   155,   136,     0,     0,
       0,     0,   137,    13,     0,     0,   138,   139,   259,   141,
     142,     0,     0,   143,    14,     0,   144,   145,   146,   147,
     148,   149,   150,   151,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   107,     0,     0,    16,     0,
       0,     0,     0,   130,     6,   131,     7,     0,     0,     8,
       0,   132,     0,   133,     9,     0,   152,     0,   153,    10,
     486,   487,     0,     0,     0,    11,   154,    12,   155,   136,
       0,     0,     0,     0,   137,    13,     0,     0,   138,   139,
     259,   141,   142,     0,     0,   143,    14,     0,   488,   145,
     146,   147,   148,   149,   150,   151,    15,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   107,     0,     0,
      16,     6,     0,     7,     0,     0,     8,     0,     0,     0,
       0,     9,     0,     0,     0,     0,    10,     0,   152,     0,
     153,     0,    11,     0,    12,     0,   136,     0,   154,     0,
     155,     0,    13,     0,     0,   138,     0,     0,   141,     0,
       0,     0,     0,    14,     0,     0,   145,   146,   147,   148,
     149,   150,   151,    15,     0,     6,     0,     7,     0,     0,
       8,     0,     0,     0,   404,     9,     0,    16,     0,     0,
      10,   224,     0,     0,     0,     0,    11,     0,    12,     0,
     136,     0,     0,   225,     0,   152,    13,   226,     0,   138,
       0,     0,   141,   227,     0,   154,     0,    14,   228,     0,
     145,   146,   147,   148,   149,   150,   151,    15,     0,     6,
       0,     7,     0,     0,     8,     0,     0,     0,     0,     9,
       0,    16,     0,     0,    10,   224,     0,     0,     0,     0,
      11,     0,    12,     0,   136,     0,     0,   225,     0,   152,
      13,   226,     0,   138,     0,     0,   141,   227,     0,   154,
       0,    14,   228,     0,   145,   146,   147,   148,   149,   150,
     151,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    39,    16,     6,     0,     7,   224,
       0,     8,     0,     0,     0,     0,     9,     0,     0,    40,
       0,    10,     0,     0,     0,   226,     0,    11,     0,    12,
      41,   136,     0,    42,    43,    44,   228,    13,    45,    46,
     138,     0,    47,   141,     0,     0,    48,     0,    14,    49,
       0,   145,   146,   147,   148,   149,   150,   151,    15,     0,
       6,     0,     7,     0,     0,     8,     0,     0,     0,     0,
       9,     0,    16,     0,     0,    10,     0,     0,     0,     0,
       0,    11,     0,    12,     0,   136,     0,     0,     0,     0,
     152,    13,   153,     0,   138,     0,     0,   141,     0,     0,
     154,     0,    14,     0,     0,   145,   146,   147,   148,   149,
     150,   151,    15,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,     0,     0,     0,
      39,     0,     6,     0,     7,     0,     0,     8,     0,     0,
       0,     0,     9,     0,   152,    40,   153,    10,     0,     0,
       0,    39,     0,    11,   154,    12,    41,     0,     0,    42,
      43,    44,     0,    13,    45,    46,    40,     0,    47,     0,
       0,     0,    48,     0,    14,    49,     0,    41,     0,     0,
      42,    43,    44,     0,    15,    45,    46,     0,     0,    47,
    -334,     0,     0,    48,  -334,     0,    49,     0,    16,     0,
    -334,     0,     0,  -334,  -335,     0,     0,     0,  -335,     0,
    -334,     0,  -334,  -334,  -335,     0,     0,  -335,  -334,     0,
       0,     0,     0,  -334,  -335,  -334,  -335,  -335,  -334,     0,
       0,   294,  -335,     0,  -334,   295,     0,  -335,     0,  -335,
       0,   296,  -335,     0,   297,     0,     0,     0,  -335,     0,
       0,   298,     0,   299,   300,     0,     0,     0,     0,   301,
       0,     0,     0,     0,   302,     0,   303,     0,     0,   304,
       0,     0,     0,     0,     0,   305
};

static const yytype_int16 yycheck[] =
{
       1,    65,    66,     1,   167,    52,   204,    30,    95,   261,
      95,   261,   158,   404,    11,    92,   518,    81,   216,    68,
     107,    62,    63,   100,   442,    70,   107,    11,    29,    70,
     444,    29,    11,    30,   418,    65,   123,    30,   123,    11,
      26,    66,     8,    27,    89,    42,    91,    13,    89,    17,
      91,   411,   101,   107,   152,    23,   154,    80,   399,    24,
      57,    62,    63,    42,    62,    63,   143,   569,    62,    70,
      42,   101,    70,    59,    71,   162,   101,   162,    57,   133,
     167,   104,    68,    69,    25,    57,   167,    80,    89,   449,
      91,    89,    71,    91,    95,   479,    90,    95,    72,    71,
      68,   522,    62,    23,   445,   206,   107,   448,   522,   107,
     211,   104,    72,   167,    96,   162,   457,    91,   100,   510,
     133,   107,   123,   109,   110,   123,   224,   225,    72,   227,
     228,   103,    62,   554,   555,   553,    77,    78,    66,   285,
     554,   555,    69,    84,    85,   131,   132,   133,    68,   570,
      72,   152,   573,   154,    66,    62,   570,   101,    62,   573,
      90,   162,    90,   204,   162,    92,   167,     3,    72,   167,
     511,   512,   158,   101,   261,   216,   103,    44,    90,   101,
     261,   167,    18,    90,    62,    72,    57,    91,    62,   101,
      31,   443,   215,    29,    72,    62,    32,    33,    34,    73,
      71,    37,    38,   204,    91,    41,   204,   261,   107,    45,
      89,    87,    48,   111,   412,   216,    62,    42,   216,    89,
      99,    97,    68,   224,   225,   226,   227,   228,   226,    99,
     284,    67,    57,   320,   133,   320,     0,    89,   336,   337,
     338,     9,    79,    57,    81,   394,    71,    99,   397,    86,
      72,    19,   107,   345,   346,   347,   348,   344,   394,    11,
     261,   397,    17,   261,   107,   101,   518,   519,   167,   519,
      74,   284,    76,     0,    68,   362,     3,   362,   133,   265,
     339,   340,   268,   341,   342,   343,   396,   310,   398,    68,
     133,    18,   349,   350,    66,   101,   282,    72,   284,   285,
      66,    68,    29,    91,   290,    32,    33,    34,   107,    44,
      37,    38,   167,    72,    41,   362,   101,   569,    45,   320,
     101,    48,   320,   101,   167,   101,   413,   313,    91,    91,
      91,   229,   413,    91,   133,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   355,    72,    72,    91,    65,   413,
      11,   362,   261,    67,   362,   101,    72,    62,   167,   101,
     447,   412,    91,   450,   101,    61,    68,    62,   101,    62,
     478,    58,    63,    59,    94,   284,   101,   101,   411,   443,
     444,    49,   101,   101,     9,    66,   107,   560,    11,   453,
      90,    62,    91,    90,    61,    61,   261,    61,   101,   307,
     308,   412,   413,    66,   412,   413,    90,   418,   261,    66,
      66,    91,   133,   409,    90,    90,   449,   413,   101,   284,
      90,   444,   519,   331,   332,    19,    91,    61,   519,    91,
     453,   284,    91,    66,   508,   107,    90,    90,    65,    91,
     527,    61,    90,    90,    68,    61,   167,   107,   444,    90,
     547,   101,   261,    67,   518,   519,   547,   453,   522,   523,
      66,   133,    62,   560,    67,   101,    65,   478,   479,   560,
     481,    90,    16,   133,    91,   284,    91,    16,    91,    90,
      90,    90,    90,   547,    65,    90,   394,    67,   552,   397,
     554,   555,    90,    90,   101,   167,   560,   101,    90,   522,
     523,    90,    65,    16,   413,   569,   570,   167,   519,   573,
     101,   519,    90,   101,   101,   344,    95,    65,    80,   104,
     313,   104,   409,    57,   110,   547,   522,   523,    62,   552,
     310,   554,   555,   104,   443,   444,   547,    71,    72,   547,
     261,   413,   547,   569,   453,   519,   377,   570,   413,   560,
     573,   547,   560,   308,   351,    89,   552,    91,   554,   555,
     413,   353,   132,   284,   560,    99,   481,   354,   352,    -1,
     306,    -1,   355,    -1,   570,    -1,    -1,   573,   443,   444,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   453,   261,
     443,   444,    -1,    -1,    -1,    60,    -1,    62,    -1,    64,
     453,   261,    -1,    -1,   413,    70,    -1,    72,    73,   518,
     519,    -1,   284,   522,   523,    80,    -1,    82,    83,    -1,
      -1,    -1,    -1,    88,   284,    -1,    91,    -1,    93,    -1,
      95,    -1,    -1,    98,   443,   444,    -1,    -1,   547,   104,
      -1,    -1,    -1,   552,   453,   554,   555,    -1,    -1,    -1,
      -1,   560,    -1,   518,   519,    -1,    -1,   522,   523,    -1,
     569,   570,    -1,    -1,   573,   518,   519,    -1,    -1,   522,
     523,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   547,    -1,    -1,    -1,    -1,   552,    -1,   554,
     555,    -1,   413,    -1,   547,   560,    -1,    -1,    -1,   552,
      -1,   554,   555,    -1,   569,   570,    -1,   560,   573,   518,
     519,    -1,    -1,   522,   523,    -1,   569,   570,    -1,    -1,
     573,    -1,   443,   444,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   453,    -1,     5,    -1,     7,    -1,   547,    10,
      11,   413,    -1,   552,    15,   554,   555,    -1,    -1,    20,
      -1,   560,    -1,   413,    -1,    26,    27,    28,    -1,    -1,
     569,   570,    -1,    -1,   573,    36,    -1,    -1,    -1,    -1,
      -1,   443,   444,    -1,    -1,    -1,    47,    -1,    -1,    -1,
     130,   453,    -1,   443,   444,    -1,    57,   137,    -1,    -1,
      -1,    -1,   142,   453,   152,    -1,   154,   518,   519,    -1,
      71,   522,   523,   153,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   547,    -1,    -1,    -1,
     152,   552,   154,   554,   555,    -1,    -1,    -1,    -1,   560,
      -1,    -1,    -1,    -1,    -1,    -1,   518,   519,   569,   570,
     522,   523,   573,    -1,    -1,    -1,    -1,    -1,   518,   519,
      -1,    -1,   522,   523,    -1,    -1,   224,   225,    -1,   227,
     228,    -1,    -1,    -1,    -1,   547,   226,    -1,    -1,    -1,
     552,    -1,   554,   555,    -1,    -1,    -1,   547,   560,    -1,
      -1,    -1,   552,    -1,   554,   555,    -1,   569,   570,    -1,
     560,   573,   224,   225,    -1,   227,   228,    -1,    -1,   569,
     570,    -1,   262,   573,     3,    -1,   266,   267,    -1,    -1,
      -1,   271,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    18,
      -1,   281,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      29,   291,    -1,    32,    33,    34,    -1,    -1,    37,    38,
      -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    48,
      -1,    -1,    -1,    -1,    -1,    -1,   316,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   336,   337,
     338,   339,   340,   341,   342,   343,    -1,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   355,    -1,    -1,
      -1,    90,    -1,    -1,    -1,    -1,   356,   357,    -1,    -1,
      -1,    -1,    -1,    -1,   336,   337,   338,   339,   340,   341,
     342,   343,    -1,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   355,     5,    -1,     7,    -1,    -1,    10,
      -1,    -1,    -1,   393,    15,    -1,    -1,    -1,    -1,    20,
      -1,    -1,    -1,    -1,   404,    26,    -1,    28,    -1,    -1,
     418,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    -1,
       5,    -1,     7,    -1,    -1,    10,    57,    -1,    -1,    -1,
      15,   441,   442,    -1,    -1,    20,   418,    -1,    -1,    -1,
      71,    26,    -1,    28,    -1,    30,   456,    -1,    -1,     3,
      -1,    36,    -1,    -1,    39,    -1,    -1,    42,    -1,    -1,
     478,   479,    47,   481,    18,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    29,    61,    -1,    32,    33,
      34,    -1,    -1,    37,    38,    -1,    71,    41,    -1,    -1,
      75,    45,    -1,    -1,    48,    -1,   478,   479,    -1,   481,
     510,    -1,    87,    -1,    89,    -1,    91,    -1,    -1,    -1,
     520,   521,    97,    67,    99,    -1,    -1,   102,    -1,     3,
       4,     5,     6,     7,     8,    -1,    10,    -1,    12,    13,
      14,    15,    -1,   543,    18,    -1,    20,    21,    22,    -1,
      -1,    -1,    26,   553,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    91,    -1,     3,
       4,     5,     6,     7,    -1,    99,    10,   101,    12,    -1,
      14,    15,    -1,    -1,    18,    -1,    20,    21,    22,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    91,    -1,     3,
       4,     5,     6,     7,    -1,    99,    10,   101,    12,    -1,
      14,    15,    -1,    -1,    18,    -1,    20,    21,    22,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    91,    -1,     3,
       4,     5,     6,     7,    -1,    99,    10,   101,    12,    -1,
      14,    15,    -1,    -1,    18,    -1,    20,    21,    22,    -1,
      -1,    -1,    26,    -1,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    91,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    99,    -1,   101,     3,     4,
       5,     6,     7,    -1,    -1,    10,    11,    12,    -1,    14,
      15,    -1,    -1,    18,    -1,    20,    21,    22,    -1,    -1,
      -1,    26,    -1,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    -1,    -1,    71,    -1,    -1,    -1,
      -1,     4,     5,     6,     7,    -1,    -1,    10,    -1,    12,
      -1,    14,    15,    -1,    89,    -1,    91,    20,    21,    22,
      -1,    -1,    -1,    26,    99,    28,   101,    30,    -1,    -1,
      -1,    -1,    35,    36,    -1,    -1,    39,    40,    41,    42,
      43,    -1,    -1,    46,    47,    -1,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,    71,    -1,
      -1,    -1,    -1,     4,     5,     6,     7,    -1,    -1,    10,
      -1,    12,    -1,    14,    15,    -1,    89,    -1,    91,    20,
      21,    22,    -1,    -1,    -1,    26,    99,    28,   101,    30,
      -1,    -1,    -1,    -1,    35,    36,    -1,    -1,    39,    40,
      41,    42,    43,    -1,    -1,    46,    47,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    -1,    -1,
      71,     5,    -1,     7,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    15,    -1,    -1,    -1,    -1,    20,    -1,    89,    -1,
      91,    -1,    26,    -1,    28,    -1,    30,    -1,    99,    -1,
     101,    -1,    36,    -1,    -1,    39,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,     5,    -1,     7,    -1,    -1,
      10,    -1,    -1,    -1,    68,    15,    -1,    71,    -1,    -1,
      20,    75,    -1,    -1,    -1,    -1,    26,    -1,    28,    -1,
      30,    -1,    -1,    87,    -1,    89,    36,    91,    -1,    39,
      -1,    -1,    42,    97,    -1,    99,    -1,    47,   102,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    -1,     5,
      -1,     7,    -1,    -1,    10,    -1,    -1,    -1,    -1,    15,
      -1,    71,    -1,    -1,    20,    75,    -1,    -1,    -1,    -1,
      26,    -1,    28,    -1,    30,    -1,    -1,    87,    -1,    89,
      36,    91,    -1,    39,    -1,    -1,    42,    97,    -1,    99,
      -1,    47,   102,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,    71,     5,    -1,     7,    75,
      -1,    10,    -1,    -1,    -1,    -1,    15,    -1,    -1,    18,
      -1,    20,    -1,    -1,    -1,    91,    -1,    26,    -1,    28,
      29,    30,    -1,    32,    33,    34,   102,    36,    37,    38,
      39,    -1,    41,    42,    -1,    -1,    45,    -1,    47,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
       5,    -1,     7,    -1,    -1,    10,    -1,    -1,    -1,    -1,
      15,    -1,    71,    -1,    -1,    20,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    28,    -1,    30,    -1,    -1,    -1,    -1,
      89,    36,    91,    -1,    39,    -1,    -1,    42,    -1,    -1,
      99,    -1,    47,    -1,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,
       3,    -1,     5,    -1,     7,    -1,    -1,    10,    -1,    -1,
      -1,    -1,    15,    -1,    89,    18,    91,    20,    -1,    -1,
      -1,     3,    -1,    26,    99,    28,    29,    -1,    -1,    32,
      33,    34,    -1,    36,    37,    38,    18,    -1,    41,    -1,
      -1,    -1,    45,    -1,    47,    48,    -1,    29,    -1,    -1,
      32,    33,    34,    -1,    57,    37,    38,    -1,    -1,    41,
      60,    -1,    -1,    45,    64,    -1,    48,    -1,    71,    -1,
      70,    -1,    -1,    73,    60,    -1,    -1,    -1,    64,    -1,
      80,    -1,    82,    83,    70,    -1,    -1,    73,    88,    -1,
      -1,    -1,    -1,    93,    80,    95,    82,    83,    98,    -1,
      -1,    60,    88,    -1,   104,    64,    -1,    93,    -1,    95,
      -1,    70,    98,    -1,    73,    -1,    -1,    -1,   104,    -1,
      -1,    80,    -1,    82,    83,    -1,    -1,    -1,    -1,    88,
      -1,    -1,    -1,    -1,    93,    -1,    95,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,   104
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    31,   107,   122,   123,   126,     5,     7,    10,    15,
      20,    26,    28,    36,    47,    57,    71,   111,   117,   118,
     119,   120,   121,     0,   124,    57,    72,   101,    72,    24,
     125,   127,   128,   129,    11,    42,   119,    11,   117,     3,
      18,    29,    32,    33,    34,    37,    38,    41,    45,    48,
     101,   130,   131,   132,   133,   134,   135,   164,   165,    72,
     101,   132,    17,    23,    68,   136,   137,   139,    11,    27,
      17,   166,   167,   103,   113,   114,   117,   113,   115,   138,
     140,   137,   139,   139,   119,   119,   115,    68,   168,    66,
     101,    66,    37,    67,   130,   135,   141,   142,   143,   148,
     149,   158,   159,   139,   169,   115,   115,    68,   177,    47,
     110,   111,   112,   113,   116,   117,   118,   160,   101,   157,
     177,    67,   134,   135,   143,   149,   165,   170,   171,   172,
       4,     6,    12,    14,    21,    22,    30,    35,    39,    40,
      41,    42,    43,    46,    49,    50,    51,    52,    53,    54,
      55,    56,    89,    91,    99,   101,   108,   109,   110,   116,
     117,   119,   131,   134,   135,   177,   178,   179,   180,   181,
     182,   183,   185,   186,   187,   189,   190,   191,   192,   194,
     201,   203,   204,   212,   213,   215,   216,   217,   218,   219,
     224,   225,   226,   230,   235,   236,   237,   238,   239,   240,
     242,   243,   258,   259,   263,   119,   151,   119,   144,   145,
     146,   151,    62,   234,   234,    91,    44,   150,   155,   101,
     101,   101,   173,   101,    75,    87,    91,    97,   102,   111,
     116,   117,   226,   236,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   261,   119,   214,   214,    41,
     183,    91,    91,   208,   261,    72,    91,    91,    72,   261,
     177,    91,   117,   235,   237,   241,   261,   241,   119,   144,
      72,    62,    72,    91,    65,   110,    67,   180,   101,   101,
      72,    62,    89,    99,    60,    64,    70,    73,    80,    82,
      83,    88,    93,    95,    98,   104,   260,   111,   113,   114,
      91,    62,   150,    66,   101,    62,    73,   150,    61,    62,
     135,   152,   153,   154,   114,   156,    68,   161,   101,   241,
     241,   111,   117,   261,   241,   241,    69,    92,   103,    87,
      97,    79,    81,    86,    25,    77,    78,    84,    85,    74,
      76,    58,    63,    94,    59,    96,   100,    65,   101,   101,
     101,    49,   131,   182,   190,   206,   209,   211,   261,   101,
     119,   261,   261,   119,   101,     9,   220,   221,   222,   261,
      90,    11,   261,    30,   228,   229,   261,   183,   144,    42,
     119,   261,   257,    62,   232,   233,   234,   232,   234,    91,
     152,    61,   145,    61,    68,   147,   174,   261,    61,   110,
      90,    66,    66,   162,   101,   231,   234,   234,    90,   241,
     241,   241,   246,   246,   247,   247,   247,   112,   248,   248,
     248,   248,   249,   249,   250,   251,   252,   253,   254,   261,
     261,    91,   101,    66,    90,    91,    90,    90,    91,    91,
      19,   223,   222,    90,    61,    90,    66,    91,    61,   261,
     231,   233,   234,   174,   231,   174,   228,    90,   147,   175,
     176,   146,   154,   114,    39,    42,   163,   178,    90,    90,
     244,    65,   101,   261,   208,   190,    21,    22,    49,   119,
     183,   184,   185,   188,   193,   202,   207,   228,    68,   195,
     177,   228,   154,   177,   183,   261,   228,    61,    90,    67,
      66,    91,    91,    67,   241,   244,   256,    90,   101,    91,
      91,    91,    65,    16,    90,   197,    90,    90,    90,   139,
     227,   147,   228,   228,   101,   205,   210,   211,   206,   261,
     261,   184,   183,     8,    13,   196,   198,   199,   200,   177,
      90,    90,    90,   101,    90,    90,   261,   262,    65,    67,
     179,   200,   101,   101,   183,   208,   184,   184,    65,   101,
      16,   205,   184,    90,   184
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   106,   107,   108,   108,   108,   108,   108,   108,   109,
     109,   110,   110,   111,   111,   111,   111,   111,   111,   111,
     111,   112,   112,   113,   114,   115,   116,   116,   117,   117,
     118,   119,   119,   120,   120,   120,   120,   121,   121,   122,
     123,   123,   124,   124,   125,   125,   126,   127,   127,   128,
     129,   130,   130,   130,   131,   131,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   133,   134,   134,
     134,   134,   135,   135,   136,   137,   138,   138,   139,   140,
     140,   141,   141,   141,   141,   142,   142,   143,   144,   144,
     145,   145,   146,   146,   147,   147,   148,   148,   148,   149,
     149,   150,   150,   151,   151,   152,   152,   153,   153,   154,
     155,   156,   156,   157,   158,   159,   159,   160,   161,   162,
     162,   163,   163,   164,   165,   166,   166,   167,   167,   168,
     169,   169,   170,   170,   170,   170,   170,   170,   171,   172,
     173,   173,   174,   175,   175,   175,   176,   176,   177,   178,
     178,   179,   179,   180,   180,   180,   181,   182,   182,   183,
     183,   183,   183,   183,   183,   184,   184,   184,   184,   184,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     185,   185,   186,   187,   188,   189,   190,   190,   190,   190,
     190,   190,   190,   191,   192,   193,   194,   195,   196,   196,
     197,   197,   198,   199,   199,   200,   200,   201,   202,   203,
     204,   205,   205,   206,   206,   207,   208,   208,   209,   209,
     210,   211,   211,   212,   212,   213,   214,   214,   215,   216,
     217,   218,   219,   219,   220,   220,   221,   221,   222,   223,
     224,   224,   225,   225,   225,   225,   225,   225,   225,   226,
     227,   227,   228,   228,   229,   229,   230,   230,   230,   230,
     231,   231,   232,   232,   233,   234,   234,   235,   235,   235,
     235,   236,   236,   236,   236,   237,   237,   238,   238,   238,
     238,   238,   239,   240,   241,   241,   241,   241,   241,   242,
     243,   244,   244,   244,   244,   245,   245,   245,   246,   246,
     246,   246,   247,   247,   247,   248,   248,   248,   248,   249,
     249,   249,   249,   249,   249,   250,   250,   250,   251,   251,
     252,   252,   253,   253,   254,   254,   255,   255,   256,   256,
     257,   257,   258,   259,   259,   259,   260,   260,   260,   260,
     260,   260,   260,   260,   260,   260,   260,   260,   261,   262,
     263,   263
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     1,
       1,     1,     2,     3,     3,     3,     3,     1,     1,     3,
       0,     1,     0,     2,     0,     2,     3,     1,     1,     3,
       5,     1,     1,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     3,
       3,     4,     0,     1,     2,     2,     1,     3,     3,     0,
       2,     1,     1,     1,     1,     1,     1,     4,     1,     3,
       1,     3,     1,     3,     1,     1,     2,     2,     3,     4,
       4,     0,     1,     4,     3,     0,     1,     1,     3,     3,
       2,     1,     3,     1,     2,     4,     5,     4,     4,     0,
       2,     5,     5,     3,     3,     0,     1,     2,     3,     3,
       0,     2,     1,     1,     1,     2,     1,     2,     1,     2,
       1,     2,     3,     0,     1,     2,     1,     3,     3,     0,
       1,     1,     2,     1,     1,     1,     2,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     5,     7,     7,     5,     4,     0,     1,
       0,     2,     2,     1,     2,     3,     2,     5,     5,     7,
       9,     0,     1,     0,     1,     9,     0,     1,     1,     1,
       1,     1,     3,     3,     5,     3,     0,     1,     3,     3,
       3,     5,     3,     4,     0,     1,     1,     2,     5,     2,
       1,     1,     1,     1,     3,     1,     1,     1,     1,     6,
       0,     1,     0,     1,     1,     3,     4,     4,     4,     4,
       0,     1,     1,     2,     3,     2,     3,     3,     3,     3,
       3,     4,     6,     6,     6,     4,     4,     1,     1,     3,
       1,     1,     2,     2,     1,     1,     2,     2,     1,     2,
       2,     1,     2,     2,     1,     5,     4,     5,     1,     3,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     1,
       3,     3,     3,     3,     3,     1,     3,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     5,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (yyscanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, yyscanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, yyscan_t yyscanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yyscanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, yyscan_t yyscanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, yyscanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, yyscan_t yyscanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], yyscanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, yyscanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, yyscan_t yyscanner)
{
  YY_USE (yyvaluep);
  YY_USE (yyscanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (yyscan_t yyscanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, yyscanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* Goal: CompilationUnit  */
#line 185 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2480 "cmDependsJavaParser.cxx"
    break;

  case 3: /* Literal: IntegerLiteral  */
#line 194 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2491 "cmDependsJavaParser.cxx"
    break;

  case 4: /* Literal: jp_FLOATINGPOINTLITERAL  */
#line 202 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2502 "cmDependsJavaParser.cxx"
    break;

  case 5: /* Literal: jp_BOOLEANLITERAL  */
#line 210 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2513 "cmDependsJavaParser.cxx"
    break;

  case 6: /* Literal: jp_CHARACTERLITERAL  */
#line 218 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2524 "cmDependsJavaParser.cxx"
    break;

  case 7: /* Literal: jp_STRINGLITERAL  */
#line 226 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2535 "cmDependsJavaParser.cxx"
    break;

  case 8: /* Literal: jp_NULLLITERAL  */
#line 234 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2546 "cmDependsJavaParser.cxx"
    break;

  case 9: /* IntegerLiteral: jp_DECIMALINTEGERLITERAL  */
#line 243 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2557 "cmDependsJavaParser.cxx"
    break;

  case 10: /* IntegerLiteral: jp_HEXINTEGERLITERAL  */
#line 251 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2568 "cmDependsJavaParser.cxx"
    break;

  case 11: /* Type: PrimitiveType  */
#line 260 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2579 "cmDependsJavaParser.cxx"
    break;

  case 12: /* Type: ReferenceType  */
#line 268 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2590 "cmDependsJavaParser.cxx"
    break;

  case 13: /* PrimitiveType: jp_BYTE_TYPE  */
#line 277 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2598 "cmDependsJavaParser.cxx"
    break;

  case 14: /* PrimitiveType: jp_SHORT_TYPE  */
#line 282 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2606 "cmDependsJavaParser.cxx"
    break;

  case 15: /* PrimitiveType: jp_INT_TYPE  */
#line 287 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2614 "cmDependsJavaParser.cxx"
    break;

  case 16: /* PrimitiveType: jp_LONG_TYPE  */
#line 292 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2622 "cmDependsJavaParser.cxx"
    break;

  case 17: /* PrimitiveType: jp_CHAR_TYPE  */
#line 297 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2630 "cmDependsJavaParser.cxx"
    break;

  case 18: /* PrimitiveType: jp_FLOAT_TYPE  */
#line 302 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2638 "cmDependsJavaParser.cxx"
    break;

  case 19: /* PrimitiveType: jp_DOUBLE_TYPE  */
#line 307 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2646 "cmDependsJavaParser.cxx"
    break;

  case 20: /* PrimitiveType: jp_BOOLEAN_TYPE  */
#line 312 "cmDependsJavaParser.y"
{
  jpElementStart(0);
}
#line 2654 "cmDependsJavaParser.cxx"
    break;

  case 21: /* ReferenceType: ClassOrInterfaceType  */
#line 318 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2665 "cmDependsJavaParser.cxx"
    break;

  case 22: /* ReferenceType: ArrayType  */
#line 326 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2676 "cmDependsJavaParser.cxx"
    break;

  case 23: /* ClassOrInterfaceType: Name  */
#line 335 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpStoreClass((yyvsp[0].str));
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2688 "cmDependsJavaParser.cxx"
    break;

  case 24: /* ClassType: ClassOrInterfaceType  */
#line 345 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2699 "cmDependsJavaParser.cxx"
    break;

  case 25: /* InterfaceType: ClassOrInterfaceType  */
#line 354 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2710 "cmDependsJavaParser.cxx"
    break;

  case 26: /* ArrayType: PrimitiveType Dims  */
#line 363 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2721 "cmDependsJavaParser.cxx"
    break;

  case 27: /* ArrayType: Name Dims  */
#line 371 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpStoreClass((yyvsp[-1].str));
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2733 "cmDependsJavaParser.cxx"
    break;

  case 28: /* Name: SimpleName  */
#line 381 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = (yyvsp[0].str);
}
#line 2742 "cmDependsJavaParser.cxx"
    break;

  case 29: /* Name: QualifiedName  */
#line 387 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = (yyvsp[0].str);
}
#line 2751 "cmDependsJavaParser.cxx"
    break;

  case 30: /* SimpleName: Identifier  */
#line 394 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = (yyvsp[0].str);
}
#line 2760 "cmDependsJavaParser.cxx"
    break;

  case 31: /* Identifier: jp_NAME  */
#line 401 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = (yyvsp[0].str);
}
#line 2769 "cmDependsJavaParser.cxx"
    break;

  case 32: /* Identifier: jp_DOLLAR jp_NAME  */
#line 407 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  (yyval.str) = (yyvsp[0].str);
}
#line 2778 "cmDependsJavaParser.cxx"
    break;

  case 33: /* QualifiedName: Name jp_DOT Identifier  */
#line 414 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->AddClassFound((yyvsp[-2].str));
  yyGetParser->UpdateCombine((yyvsp[-2].str), (yyvsp[0].str));
  yyGetParser->DeallocateParserType(&((yyvsp[-2].str)));
  (yyval.str) = const_cast<char*>(yyGetParser->GetCurrentCombine());
}
#line 2790 "cmDependsJavaParser.cxx"
    break;

  case 34: /* QualifiedName: Name jp_DOT jp_CLASS  */
#line 423 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpStoreClass((yyvsp[-2].str));
  jpCheckEmpty(3);
  yyGetParser->SetCurrentCombine("");
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2803 "cmDependsJavaParser.cxx"
    break;

  case 35: /* QualifiedName: Name jp_DOT jp_THIS  */
#line 433 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpStoreClass((yyvsp[-2].str));
  yyGetParser->SetCurrentCombine("");
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2816 "cmDependsJavaParser.cxx"
    break;

  case 36: /* QualifiedName: SimpleType jp_DOT jp_CLASS  */
#line 443 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2827 "cmDependsJavaParser.cxx"
    break;

  case 37: /* SimpleType: PrimitiveType  */
#line 452 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2838 "cmDependsJavaParser.cxx"
    break;

  case 38: /* SimpleType: jp_VOID  */
#line 460 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2849 "cmDependsJavaParser.cxx"
    break;

  case 39: /* CompilationUnit: PackageDeclarationopt ImportDeclarations TypeDeclarations  */
#line 469 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2860 "cmDependsJavaParser.cxx"
    break;

  case 40: /* PackageDeclarationopt: %empty  */
#line 477 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2870 "cmDependsJavaParser.cxx"
    break;

  case 41: /* PackageDeclarationopt: PackageDeclaration  */
#line 484 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2881 "cmDependsJavaParser.cxx"
    break;

  case 42: /* ImportDeclarations: %empty  */
#line 492 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2891 "cmDependsJavaParser.cxx"
    break;

  case 43: /* ImportDeclarations: ImportDeclarations ImportDeclaration  */
#line 499 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2902 "cmDependsJavaParser.cxx"
    break;

  case 44: /* TypeDeclarations: %empty  */
#line 507 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2912 "cmDependsJavaParser.cxx"
    break;

  case 45: /* TypeDeclarations: TypeDeclarations TypeDeclaration  */
#line 514 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2923 "cmDependsJavaParser.cxx"
    break;

  case 46: /* PackageDeclaration: jp_PACKAGE Name jp_SEMICOL  */
#line 523 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->SetCurrentPackage((yyvsp[-1].str));
  yyGetParser->DeallocateParserType(&((yyvsp[-1].str)));
  yyGetParser->SetCurrentCombine("");
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2937 "cmDependsJavaParser.cxx"
    break;

  case 47: /* ImportDeclaration: SingleTypeImportDeclaration  */
#line 535 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2948 "cmDependsJavaParser.cxx"
    break;

  case 48: /* ImportDeclaration: TypeImportOnDemandDeclaration  */
#line 543 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2959 "cmDependsJavaParser.cxx"
    break;

  case 49: /* SingleTypeImportDeclaration: jp_IMPORT Name jp_SEMICOL  */
#line 552 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->AddPackagesImport((yyvsp[-1].str));
  yyGetParser->DeallocateParserType(&((yyvsp[-1].str)));
  yyGetParser->SetCurrentCombine("");
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2973 "cmDependsJavaParser.cxx"
    break;

  case 50: /* TypeImportOnDemandDeclaration: jp_IMPORT Name jp_DOT jp_TIMES jp_SEMICOL  */
#line 564 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  std::string str = (yyvsp[-3].str);
  str += ".*";
  yyGetParser->AddPackagesImport(str.c_str());
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  yyGetParser->SetCurrentCombine("");
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2988 "cmDependsJavaParser.cxx"
    break;

  case 51: /* TypeDeclaration: ClassDeclaration  */
#line 577 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 2999 "cmDependsJavaParser.cxx"
    break;

  case 52: /* TypeDeclaration: InterfaceDeclaration  */
#line 585 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3010 "cmDependsJavaParser.cxx"
    break;

  case 53: /* TypeDeclaration: jp_SEMICOL  */
#line 593 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3021 "cmDependsJavaParser.cxx"
    break;

  case 54: /* Modifiers: Modifier  */
#line 602 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3032 "cmDependsJavaParser.cxx"
    break;

  case 55: /* Modifiers: Modifiers Modifier  */
#line 610 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3043 "cmDependsJavaParser.cxx"
    break;

  case 67: /* ClassHeader: Modifiersopt jp_CLASS Identifier  */
#line 625 "cmDependsJavaParser.y"
{
  yyGetParser->StartClass((yyvsp[0].str));
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(3);
}
#line 3054 "cmDependsJavaParser.cxx"
    break;

  case 68: /* ClassDeclaration: ClassHeader ClassBody  */
#line 635 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
#line 3066 "cmDependsJavaParser.cxx"
    break;

  case 69: /* ClassDeclaration: ClassHeader Interfaces ClassBody  */
#line 644 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
#line 3078 "cmDependsJavaParser.cxx"
    break;

  case 70: /* ClassDeclaration: ClassHeader Super ClassBody  */
#line 653 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
#line 3090 "cmDependsJavaParser.cxx"
    break;

  case 71: /* ClassDeclaration: ClassHeader Super Interfaces ClassBody  */
#line 662 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
#line 3102 "cmDependsJavaParser.cxx"
    break;

  case 72: /* Modifiersopt: %empty  */
#line 671 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3112 "cmDependsJavaParser.cxx"
    break;

  case 73: /* Modifiersopt: Modifiers  */
#line 678 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3123 "cmDependsJavaParser.cxx"
    break;

  case 74: /* Super: jp_EXTENDS ClassType  */
#line 687 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3134 "cmDependsJavaParser.cxx"
    break;

  case 75: /* Interfaces: jp_IMPLEMENTS InterfaceTypeList  */
#line 696 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3145 "cmDependsJavaParser.cxx"
    break;

  case 76: /* InterfaceTypeList: InterfaceType  */
#line 705 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3156 "cmDependsJavaParser.cxx"
    break;

  case 77: /* InterfaceTypeList: InterfaceTypeList jp_COMMA InterfaceType  */
#line 713 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3167 "cmDependsJavaParser.cxx"
    break;

  case 78: /* ClassBody: jp_CURLYSTART ClassBodyDeclarations jp_CURLYEND  */
#line 722 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3178 "cmDependsJavaParser.cxx"
    break;

  case 79: /* ClassBodyDeclarations: %empty  */
#line 730 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3188 "cmDependsJavaParser.cxx"
    break;

  case 80: /* ClassBodyDeclarations: ClassBodyDeclarations ClassBodyDeclaration  */
#line 737 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3199 "cmDependsJavaParser.cxx"
    break;

  case 81: /* ClassBodyDeclaration: ClassMemberDeclaration  */
#line 746 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3210 "cmDependsJavaParser.cxx"
    break;

  case 82: /* ClassBodyDeclaration: StaticInitializer  */
#line 754 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3221 "cmDependsJavaParser.cxx"
    break;

  case 83: /* ClassBodyDeclaration: ConstructorDeclaration  */
#line 762 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3232 "cmDependsJavaParser.cxx"
    break;

  case 84: /* ClassBodyDeclaration: TypeDeclaration  */
#line 770 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3243 "cmDependsJavaParser.cxx"
    break;

  case 85: /* ClassMemberDeclaration: FieldDeclaration  */
#line 779 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3254 "cmDependsJavaParser.cxx"
    break;

  case 86: /* ClassMemberDeclaration: MethodDeclaration  */
#line 787 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3265 "cmDependsJavaParser.cxx"
    break;

  case 87: /* FieldDeclaration: Modifiersopt Type VariableDeclarators jp_SEMICOL  */
#line 796 "cmDependsJavaParser.y"
{
  jpElementStart(4);
}
#line 3273 "cmDependsJavaParser.cxx"
    break;

  case 88: /* VariableDeclarators: VariableDeclarator  */
#line 802 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3284 "cmDependsJavaParser.cxx"
    break;

  case 89: /* VariableDeclarators: VariableDeclarators jp_COMMA VariableDeclarator  */
#line 810 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3295 "cmDependsJavaParser.cxx"
    break;

  case 90: /* VariableDeclarator: VariableDeclaratorId  */
#line 819 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3306 "cmDependsJavaParser.cxx"
    break;

  case 91: /* VariableDeclarator: VariableDeclaratorId jp_EQUALS VariableInitializer  */
#line 827 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3317 "cmDependsJavaParser.cxx"
    break;

  case 92: /* VariableDeclaratorId: Identifier  */
#line 836 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3329 "cmDependsJavaParser.cxx"
    break;

  case 93: /* VariableDeclaratorId: VariableDeclaratorId jp_BRACKETSTART jp_BRACKETEND  */
#line 845 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3340 "cmDependsJavaParser.cxx"
    break;

  case 94: /* VariableInitializer: Expression  */
#line 854 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3351 "cmDependsJavaParser.cxx"
    break;

  case 95: /* VariableInitializer: ArrayInitializer  */
#line 862 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3362 "cmDependsJavaParser.cxx"
    break;

  case 96: /* MethodDeclaration: MethodHeader jp_SEMICOL  */
#line 871 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3373 "cmDependsJavaParser.cxx"
    break;

  case 97: /* MethodDeclaration: MethodHeader MethodBody  */
#line 879 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3384 "cmDependsJavaParser.cxx"
    break;

  case 98: /* MethodDeclaration: MethodHeader MethodBody jp_SEMICOL  */
#line 887 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3395 "cmDependsJavaParser.cxx"
    break;

  case 99: /* MethodHeader: Modifiersopt Type MethodDeclarator Throwsopt  */
#line 896 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3407 "cmDependsJavaParser.cxx"
    break;

  case 100: /* MethodHeader: Modifiersopt jp_VOID MethodDeclarator Throwsopt  */
#line 905 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3419 "cmDependsJavaParser.cxx"
    break;

  case 101: /* Throwsopt: %empty  */
#line 914 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3430 "cmDependsJavaParser.cxx"
    break;

  case 102: /* Throwsopt: Throws  */
#line 922 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3442 "cmDependsJavaParser.cxx"
    break;

  case 103: /* MethodDeclarator: Identifier jp_PARESTART FormalParameterListopt jp_PAREEND  */
#line 932 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3455 "cmDependsJavaParser.cxx"
    break;

  case 104: /* MethodDeclarator: MethodDeclarator jp_BRACKETSTART jp_BRACKETEND  */
#line 942 "cmDependsJavaParser.y"
{
  jpElementStart(3);

}
#line 3464 "cmDependsJavaParser.cxx"
    break;

  case 105: /* FormalParameterListopt: %empty  */
#line 948 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3475 "cmDependsJavaParser.cxx"
    break;

  case 107: /* FormalParameterList: FormalParameter  */
#line 959 "cmDependsJavaParser.y"
{
  jpElementStart(1);

}
#line 3484 "cmDependsJavaParser.cxx"
    break;

  case 108: /* FormalParameterList: FormalParameterList jp_COMMA FormalParameter  */
#line 965 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3496 "cmDependsJavaParser.cxx"
    break;

  case 109: /* FormalParameter: Modifiersopt Type VariableDeclaratorId  */
#line 975 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3508 "cmDependsJavaParser.cxx"
    break;

  case 110: /* Throws: jp_THROWS ClassTypeList  */
#line 985 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3520 "cmDependsJavaParser.cxx"
    break;

  case 111: /* ClassTypeList: ClassType  */
#line 995 "cmDependsJavaParser.y"
{
  jpElementStart(1);

}
#line 3529 "cmDependsJavaParser.cxx"
    break;

  case 112: /* ClassTypeList: ClassTypeList jp_COMMA ClassType  */
#line 1001 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3541 "cmDependsJavaParser.cxx"
    break;

  case 113: /* MethodBody: Block  */
#line 1011 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3553 "cmDependsJavaParser.cxx"
    break;

  case 114: /* StaticInitializer: jp_STATIC Block  */
#line 1021 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3565 "cmDependsJavaParser.cxx"
    break;

  case 115: /* ConstructorDeclaration: Modifiersopt ConstructorDeclarator Throwsopt ConstructorBody  */
#line 1031 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3577 "cmDependsJavaParser.cxx"
    break;

  case 116: /* ConstructorDeclaration: Modifiersopt ConstructorDeclarator Throwsopt ConstructorBody jp_SEMICOL  */
#line 1040 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3589 "cmDependsJavaParser.cxx"
    break;

  case 117: /* ConstructorDeclarator: SimpleName jp_PARESTART FormalParameterListopt jp_PAREEND  */
#line 1050 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3602 "cmDependsJavaParser.cxx"
    break;

  case 118: /* ConstructorBody: jp_CURLYSTART ExplicitConstructorInvocationopt BlockStatementsopt jp_CURLYEND  */
#line 1061 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3614 "cmDependsJavaParser.cxx"
    break;

  case 119: /* ExplicitConstructorInvocationopt: %empty  */
#line 1070 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3625 "cmDependsJavaParser.cxx"
    break;

  case 120: /* ExplicitConstructorInvocationopt: ExplicitConstructorInvocationopt ExplicitConstructorInvocation  */
#line 1078 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3637 "cmDependsJavaParser.cxx"
    break;

  case 121: /* ExplicitConstructorInvocation: jp_THIS jp_PARESTART ArgumentListopt jp_PAREEND jp_SEMICOL  */
#line 1088 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3649 "cmDependsJavaParser.cxx"
    break;

  case 122: /* ExplicitConstructorInvocation: jp_SUPER jp_PARESTART ArgumentListopt jp_PAREEND jp_SEMICOL  */
#line 1097 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3661 "cmDependsJavaParser.cxx"
    break;

  case 123: /* InterfaceHeader: Modifiersopt jp_INTERFACE Identifier  */
#line 1107 "cmDependsJavaParser.y"
{
  yyGetParser->StartClass((yyvsp[0].str));
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(3);
}
#line 3672 "cmDependsJavaParser.cxx"
    break;

  case 124: /* InterfaceDeclaration: InterfaceHeader ExtendsInterfacesopt InterfaceBody  */
#line 1116 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
#line 3684 "cmDependsJavaParser.cxx"
    break;

  case 125: /* ExtendsInterfacesopt: %empty  */
#line 1125 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");
}
#line 3694 "cmDependsJavaParser.cxx"
    break;

  case 126: /* ExtendsInterfacesopt: ExtendsInterfaces  */
#line 1132 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3706 "cmDependsJavaParser.cxx"
    break;

  case 127: /* ExtendsInterfaces: jp_EXTENDS InterfaceType  */
#line 1142 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3718 "cmDependsJavaParser.cxx"
    break;

  case 128: /* ExtendsInterfaces: ExtendsInterfaces jp_COMMA InterfaceType  */
#line 1151 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3730 "cmDependsJavaParser.cxx"
    break;

  case 129: /* InterfaceBody: jp_CURLYSTART InterfaceMemberDeclarations jp_CURLYEND  */
#line 1161 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3742 "cmDependsJavaParser.cxx"
    break;

  case 130: /* InterfaceMemberDeclarations: %empty  */
#line 1170 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3753 "cmDependsJavaParser.cxx"
    break;

  case 131: /* InterfaceMemberDeclarations: InterfaceMemberDeclarations InterfaceMemberDeclaration  */
#line 1178 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3764 "cmDependsJavaParser.cxx"
    break;

  case 132: /* InterfaceMemberDeclaration: ConstantDeclaration  */
#line 1187 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3776 "cmDependsJavaParser.cxx"
    break;

  case 133: /* InterfaceMemberDeclaration: AbstractMethodDeclaration  */
#line 1196 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3788 "cmDependsJavaParser.cxx"
    break;

  case 134: /* InterfaceMemberDeclaration: ClassDeclaration  */
#line 1205 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3800 "cmDependsJavaParser.cxx"
    break;

  case 135: /* InterfaceMemberDeclaration: ClassDeclaration jp_SEMICOL  */
#line 1214 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3811 "cmDependsJavaParser.cxx"
    break;

  case 136: /* InterfaceMemberDeclaration: InterfaceDeclaration  */
#line 1222 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3823 "cmDependsJavaParser.cxx"
    break;

  case 137: /* InterfaceMemberDeclaration: InterfaceDeclaration jp_SEMICOL  */
#line 1231 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3834 "cmDependsJavaParser.cxx"
    break;

  case 138: /* ConstantDeclaration: FieldDeclaration  */
#line 1240 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3846 "cmDependsJavaParser.cxx"
    break;

  case 139: /* AbstractMethodDeclaration: MethodHeader Semicols  */
#line 1250 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3858 "cmDependsJavaParser.cxx"
    break;

  case 140: /* Semicols: jp_SEMICOL  */
#line 1260 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3870 "cmDependsJavaParser.cxx"
    break;

  case 141: /* Semicols: Semicols jp_SEMICOL  */
#line 1269 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3882 "cmDependsJavaParser.cxx"
    break;

  case 142: /* ArrayInitializer: jp_CURLYSTART VariableInitializersOptional jp_CURLYEND  */
#line 1279 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3894 "cmDependsJavaParser.cxx"
    break;

  case 143: /* VariableInitializersOptional: %empty  */
#line 1288 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3905 "cmDependsJavaParser.cxx"
    break;

  case 144: /* VariableInitializersOptional: VariableInitializers  */
#line 1296 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3917 "cmDependsJavaParser.cxx"
    break;

  case 145: /* VariableInitializersOptional: VariableInitializers jp_COMMA  */
#line 1305 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3929 "cmDependsJavaParser.cxx"
    break;

  case 146: /* VariableInitializers: VariableInitializer  */
#line 1315 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3941 "cmDependsJavaParser.cxx"
    break;

  case 147: /* VariableInitializers: VariableInitializers jp_COMMA VariableInitializer  */
#line 1324 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3953 "cmDependsJavaParser.cxx"
    break;

  case 148: /* Block: jp_CURLYSTART BlockStatementsopt jp_CURLYEND  */
#line 1334 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3964 "cmDependsJavaParser.cxx"
    break;

  case 149: /* BlockStatementsopt: %empty  */
#line 1342 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3975 "cmDependsJavaParser.cxx"
    break;

  case 150: /* BlockStatementsopt: BlockStatements  */
#line 1350 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3987 "cmDependsJavaParser.cxx"
    break;

  case 151: /* BlockStatements: BlockStatement  */
#line 1360 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 3999 "cmDependsJavaParser.cxx"
    break;

  case 152: /* BlockStatements: BlockStatements BlockStatement  */
#line 1369 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4011 "cmDependsJavaParser.cxx"
    break;

  case 153: /* BlockStatement: LocalVariableDeclarationStatement  */
#line 1379 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4023 "cmDependsJavaParser.cxx"
    break;

  case 154: /* BlockStatement: Statement  */
#line 1388 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4035 "cmDependsJavaParser.cxx"
    break;

  case 155: /* BlockStatement: ClassDeclaration  */
#line 1397 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4047 "cmDependsJavaParser.cxx"
    break;

  case 156: /* LocalVariableDeclarationStatement: LocalVariableDeclaration jp_SEMICOL  */
#line 1407 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4059 "cmDependsJavaParser.cxx"
    break;

  case 157: /* LocalVariableDeclaration: Modifiers Type VariableDeclarators  */
#line 1417 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4071 "cmDependsJavaParser.cxx"
    break;

  case 158: /* LocalVariableDeclaration: Type VariableDeclarators  */
#line 1426 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4083 "cmDependsJavaParser.cxx"
    break;

  case 159: /* Statement: StatementWithoutTrailingSubstatement  */
#line 1436 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4095 "cmDependsJavaParser.cxx"
    break;

  case 160: /* Statement: LabeledStatement  */
#line 1445 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4107 "cmDependsJavaParser.cxx"
    break;

  case 161: /* Statement: IfThenStatement  */
#line 1454 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4119 "cmDependsJavaParser.cxx"
    break;

  case 162: /* Statement: IfThenElseStatement  */
#line 1463 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4131 "cmDependsJavaParser.cxx"
    break;

  case 163: /* Statement: WhileStatement  */
#line 1472 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4143 "cmDependsJavaParser.cxx"
    break;

  case 164: /* Statement: ForStatement  */
#line 1481 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4155 "cmDependsJavaParser.cxx"
    break;

  case 165: /* StatementNoShortIf: StatementWithoutTrailingSubstatement  */
#line 1491 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4167 "cmDependsJavaParser.cxx"
    break;

  case 166: /* StatementNoShortIf: LabeledStatementNoShortIf  */
#line 1500 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4179 "cmDependsJavaParser.cxx"
    break;

  case 167: /* StatementNoShortIf: IfThenElseStatementNoShortIf  */
#line 1509 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4191 "cmDependsJavaParser.cxx"
    break;

  case 168: /* StatementNoShortIf: WhileStatementNoShortIf  */
#line 1518 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4203 "cmDependsJavaParser.cxx"
    break;

  case 169: /* StatementNoShortIf: ForStatementNoShortIf  */
#line 1527 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4215 "cmDependsJavaParser.cxx"
    break;

  case 170: /* StatementWithoutTrailingSubstatement: Block  */
#line 1537 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4227 "cmDependsJavaParser.cxx"
    break;

  case 171: /* StatementWithoutTrailingSubstatement: EmptyStatement  */
#line 1546 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4239 "cmDependsJavaParser.cxx"
    break;

  case 172: /* StatementWithoutTrailingSubstatement: ExpressionStatement  */
#line 1555 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4251 "cmDependsJavaParser.cxx"
    break;

  case 173: /* StatementWithoutTrailingSubstatement: SwitchStatement  */
#line 1564 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4263 "cmDependsJavaParser.cxx"
    break;

  case 174: /* StatementWithoutTrailingSubstatement: DoStatement  */
#line 1573 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4275 "cmDependsJavaParser.cxx"
    break;

  case 175: /* StatementWithoutTrailingSubstatement: BreakStatement  */
#line 1582 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4287 "cmDependsJavaParser.cxx"
    break;

  case 176: /* StatementWithoutTrailingSubstatement: ContinueStatement  */
#line 1591 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4299 "cmDependsJavaParser.cxx"
    break;

  case 177: /* StatementWithoutTrailingSubstatement: ReturnStatement  */
#line 1600 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4311 "cmDependsJavaParser.cxx"
    break;

  case 178: /* StatementWithoutTrailingSubstatement: SynchronizedStatement  */
#line 1609 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4323 "cmDependsJavaParser.cxx"
    break;

  case 179: /* StatementWithoutTrailingSubstatement: ThrowStatement  */
#line 1618 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4335 "cmDependsJavaParser.cxx"
    break;

  case 180: /* StatementWithoutTrailingSubstatement: TryStatement  */
#line 1627 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4347 "cmDependsJavaParser.cxx"
    break;

  case 181: /* StatementWithoutTrailingSubstatement: AssertStatement  */
#line 1636 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4359 "cmDependsJavaParser.cxx"
    break;

  case 182: /* EmptyStatement: jp_SEMICOL  */
#line 1646 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4371 "cmDependsJavaParser.cxx"
    break;

  case 183: /* LabeledStatement: Identifier jp_COLON Statement  */
#line 1656 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[-2].str)));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4384 "cmDependsJavaParser.cxx"
    break;

  case 184: /* LabeledStatementNoShortIf: Identifier jp_COLON StatementNoShortIf  */
#line 1667 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4396 "cmDependsJavaParser.cxx"
    break;

  case 185: /* ExpressionStatement: StatementExpression jp_SEMICOL  */
#line 1677 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4408 "cmDependsJavaParser.cxx"
    break;

  case 186: /* StatementExpression: Assignment  */
#line 1687 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4420 "cmDependsJavaParser.cxx"
    break;

  case 187: /* StatementExpression: PreIncrementExpression  */
#line 1696 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4432 "cmDependsJavaParser.cxx"
    break;

  case 188: /* StatementExpression: PreDecrementExpression  */
#line 1705 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4444 "cmDependsJavaParser.cxx"
    break;

  case 189: /* StatementExpression: PostIncrementExpression  */
#line 1714 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4456 "cmDependsJavaParser.cxx"
    break;

  case 190: /* StatementExpression: PostDecrementExpression  */
#line 1723 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4468 "cmDependsJavaParser.cxx"
    break;

  case 191: /* StatementExpression: MethodInvocation  */
#line 1732 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4480 "cmDependsJavaParser.cxx"
    break;

  case 192: /* StatementExpression: ClassInstanceCreationExpression  */
#line 1741 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4492 "cmDependsJavaParser.cxx"
    break;

  case 193: /* IfThenStatement: jp_IF jp_PARESTART Expression jp_PAREEND Statement  */
#line 1751 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4504 "cmDependsJavaParser.cxx"
    break;

  case 194: /* IfThenElseStatement: jp_IF jp_PARESTART Expression jp_PAREEND StatementNoShortIf jp_ELSE Statement  */
#line 1761 "cmDependsJavaParser.y"
{
  jpElementStart(7);
  jpCheckEmpty(7);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4516 "cmDependsJavaParser.cxx"
    break;

  case 195: /* IfThenElseStatementNoShortIf: jp_IF jp_PARESTART Expression jp_PAREEND StatementNoShortIf jp_ELSE StatementNoShortIf  */
#line 1771 "cmDependsJavaParser.y"
{
  jpElementStart(7);
  jpCheckEmpty(7);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4528 "cmDependsJavaParser.cxx"
    break;

  case 196: /* SwitchStatement: jp_SWITCH jp_PARESTART Expression jp_PAREEND SwitchBlock  */
#line 1781 "cmDependsJavaParser.y"
{
  jpElementStart(5);

}
#line 4537 "cmDependsJavaParser.cxx"
    break;

  case 197: /* SwitchBlock: jp_CURLYSTART SwitchBlockStatementGroups SwitchLabelsopt jp_CURLYEND  */
#line 1788 "cmDependsJavaParser.y"
{
  jpElementStart(4);

}
#line 4546 "cmDependsJavaParser.cxx"
    break;

  case 198: /* SwitchLabelsopt: %empty  */
#line 1794 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4557 "cmDependsJavaParser.cxx"
    break;

  case 199: /* SwitchLabelsopt: SwitchLabels  */
#line 1802 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4569 "cmDependsJavaParser.cxx"
    break;

  case 200: /* SwitchBlockStatementGroups: %empty  */
#line 1811 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4580 "cmDependsJavaParser.cxx"
    break;

  case 201: /* SwitchBlockStatementGroups: SwitchBlockStatementGroups SwitchBlockStatementGroup  */
#line 1819 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4592 "cmDependsJavaParser.cxx"
    break;

  case 202: /* SwitchBlockStatementGroup: SwitchLabels BlockStatements  */
#line 1829 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4604 "cmDependsJavaParser.cxx"
    break;

  case 203: /* SwitchLabels: SwitchLabel  */
#line 1839 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4616 "cmDependsJavaParser.cxx"
    break;

  case 204: /* SwitchLabels: SwitchLabels SwitchLabel  */
#line 1848 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4628 "cmDependsJavaParser.cxx"
    break;

  case 205: /* SwitchLabel: jp_CASE ConstantExpression jp_COLON  */
#line 1858 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4640 "cmDependsJavaParser.cxx"
    break;

  case 206: /* SwitchLabel: jp_DEFAULT jp_COLON  */
#line 1867 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4652 "cmDependsJavaParser.cxx"
    break;

  case 207: /* WhileStatement: jp_WHILE jp_PARESTART Expression jp_PAREEND Statement  */
#line 1877 "cmDependsJavaParser.y"
{
  jpElementStart(5);

}
#line 4661 "cmDependsJavaParser.cxx"
    break;

  case 208: /* WhileStatementNoShortIf: jp_WHILE jp_PARESTART Expression jp_PAREEND StatementNoShortIf  */
#line 1884 "cmDependsJavaParser.y"
{
  jpElementStart(5);

}
#line 4670 "cmDependsJavaParser.cxx"
    break;

  case 209: /* DoStatement: jp_DO Statement jp_WHILE jp_PARESTART Expression jp_PAREEND jp_SEMICOL  */
#line 1891 "cmDependsJavaParser.y"
{
  jpElementStart(7);

}
#line 4679 "cmDependsJavaParser.cxx"
    break;

  case 210: /* ForStatement: jp_FOR jp_PARESTART ForInitopt jp_SEMICOL Expressionopt jp_SEMICOL ForUpdateopt jp_PAREEND Statement  */
#line 1899 "cmDependsJavaParser.y"
{
  jpElementStart(9);

}
#line 4688 "cmDependsJavaParser.cxx"
    break;

  case 211: /* ForUpdateopt: %empty  */
#line 1905 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4699 "cmDependsJavaParser.cxx"
    break;

  case 212: /* ForUpdateopt: ForUpdate  */
#line 1913 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4711 "cmDependsJavaParser.cxx"
    break;

  case 213: /* ForInitopt: %empty  */
#line 1922 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4722 "cmDependsJavaParser.cxx"
    break;

  case 214: /* ForInitopt: ForInit  */
#line 1930 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4734 "cmDependsJavaParser.cxx"
    break;

  case 215: /* ForStatementNoShortIf: jp_FOR jp_PARESTART ForInitopt jp_SEMICOL Expressionopt jp_SEMICOL ForUpdateopt jp_PAREEND StatementNoShortIf  */
#line 1941 "cmDependsJavaParser.y"
{
  jpElementStart(9);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4745 "cmDependsJavaParser.cxx"
    break;

  case 216: /* Expressionopt: %empty  */
#line 1949 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4756 "cmDependsJavaParser.cxx"
    break;

  case 217: /* Expressionopt: Expression  */
#line 1957 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4768 "cmDependsJavaParser.cxx"
    break;

  case 218: /* ForInit: StatementExpressionList  */
#line 1967 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4780 "cmDependsJavaParser.cxx"
    break;

  case 219: /* ForInit: LocalVariableDeclaration  */
#line 1976 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4792 "cmDependsJavaParser.cxx"
    break;

  case 220: /* ForUpdate: StatementExpressionList  */
#line 1986 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4804 "cmDependsJavaParser.cxx"
    break;

  case 221: /* StatementExpressionList: StatementExpression  */
#line 1996 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4816 "cmDependsJavaParser.cxx"
    break;

  case 222: /* StatementExpressionList: StatementExpressionList jp_COMMA StatementExpression  */
#line 2005 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4828 "cmDependsJavaParser.cxx"
    break;

  case 223: /* AssertStatement: jp_ASSERT Expression jp_SEMICOL  */
#line 2015 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4840 "cmDependsJavaParser.cxx"
    break;

  case 224: /* AssertStatement: jp_ASSERT Expression jp_COLON Expression jp_SEMICOL  */
#line 2024 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4852 "cmDependsJavaParser.cxx"
    break;

  case 225: /* BreakStatement: jp_BREAK Identifieropt jp_SEMICOL  */
#line 2034 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[-1].str)));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4865 "cmDependsJavaParser.cxx"
    break;

  case 226: /* Identifieropt: %empty  */
#line 2044 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4876 "cmDependsJavaParser.cxx"
    break;

  case 227: /* Identifieropt: Identifier  */
#line 2052 "cmDependsJavaParser.y"
{
  jpElementStart(1);

}
#line 4885 "cmDependsJavaParser.cxx"
    break;

  case 228: /* ContinueStatement: jp_CONTINUE Identifieropt jp_SEMICOL  */
#line 2059 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[-1].str)));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4898 "cmDependsJavaParser.cxx"
    break;

  case 229: /* ReturnStatement: jp_RETURN Expressionopt jp_SEMICOL  */
#line 2070 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4910 "cmDependsJavaParser.cxx"
    break;

  case 230: /* ThrowStatement: jp_THROW Expression jp_SEMICOL  */
#line 2080 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4922 "cmDependsJavaParser.cxx"
    break;

  case 231: /* SynchronizedStatement: jp_SYNCHRONIZED jp_PARESTART Expression jp_PAREEND Block  */
#line 2090 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4934 "cmDependsJavaParser.cxx"
    break;

  case 232: /* TryStatement: jp_TRY Block Catches  */
#line 2100 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4946 "cmDependsJavaParser.cxx"
    break;

  case 233: /* TryStatement: jp_TRY Block Catchesopt Finally  */
#line 2109 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4958 "cmDependsJavaParser.cxx"
    break;

  case 234: /* Catchesopt: %empty  */
#line 2118 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4969 "cmDependsJavaParser.cxx"
    break;

  case 235: /* Catchesopt: Catches  */
#line 2126 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4981 "cmDependsJavaParser.cxx"
    break;

  case 236: /* Catches: CatchClause  */
#line 2136 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 4993 "cmDependsJavaParser.cxx"
    break;

  case 237: /* Catches: Catches CatchClause  */
#line 2145 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5005 "cmDependsJavaParser.cxx"
    break;

  case 238: /* CatchClause: jp_CATCH jp_PARESTART FormalParameter jp_PAREEND Block  */
#line 2155 "cmDependsJavaParser.y"
{
  jpElementStart(5);

}
#line 5014 "cmDependsJavaParser.cxx"
    break;

  case 239: /* Finally: jp_FINALLY Block  */
#line 2162 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5026 "cmDependsJavaParser.cxx"
    break;

  case 240: /* Primary: PrimaryNoNewArray  */
#line 2172 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5038 "cmDependsJavaParser.cxx"
    break;

  case 241: /* Primary: ArrayCreationExpression  */
#line 2181 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5050 "cmDependsJavaParser.cxx"
    break;

  case 242: /* PrimaryNoNewArray: Literal  */
#line 2191 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5062 "cmDependsJavaParser.cxx"
    break;

  case 243: /* PrimaryNoNewArray: jp_THIS  */
#line 2200 "cmDependsJavaParser.y"
{
  jpElementStart(1);

}
#line 5071 "cmDependsJavaParser.cxx"
    break;

  case 244: /* PrimaryNoNewArray: jp_PARESTART Expression jp_PAREEND  */
#line 2206 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5083 "cmDependsJavaParser.cxx"
    break;

  case 245: /* PrimaryNoNewArray: ClassInstanceCreationExpression  */
#line 2215 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5095 "cmDependsJavaParser.cxx"
    break;

  case 246: /* PrimaryNoNewArray: FieldAccess  */
#line 2224 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5107 "cmDependsJavaParser.cxx"
    break;

  case 247: /* PrimaryNoNewArray: MethodInvocation  */
#line 2233 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5119 "cmDependsJavaParser.cxx"
    break;

  case 248: /* PrimaryNoNewArray: ArrayAccess  */
#line 2242 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5131 "cmDependsJavaParser.cxx"
    break;

  case 249: /* ClassInstanceCreationExpression: New ClassType jp_PARESTART ArgumentListopt jp_PAREEND ClassBodyOpt  */
#line 2252 "cmDependsJavaParser.y"
{
  jpElementStart(6);
  jpCheckEmpty(6);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5143 "cmDependsJavaParser.cxx"
    break;

  case 250: /* ClassBodyOpt: %empty  */
#line 2261 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5154 "cmDependsJavaParser.cxx"
    break;

  case 251: /* ClassBodyOpt: ClassBody  */
#line 2269 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5166 "cmDependsJavaParser.cxx"
    break;

  case 252: /* ArgumentListopt: %empty  */
#line 2278 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5177 "cmDependsJavaParser.cxx"
    break;

  case 253: /* ArgumentListopt: ArgumentList  */
#line 2286 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5189 "cmDependsJavaParser.cxx"
    break;

  case 254: /* ArgumentList: Expression  */
#line 2296 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5201 "cmDependsJavaParser.cxx"
    break;

  case 255: /* ArgumentList: ArgumentList jp_COMMA Expression  */
#line 2305 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5213 "cmDependsJavaParser.cxx"
    break;

  case 256: /* ArrayCreationExpression: New PrimitiveType DimExprs Dimsopt  */
#line 2315 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5225 "cmDependsJavaParser.cxx"
    break;

  case 257: /* ArrayCreationExpression: New ClassOrInterfaceType DimExprs Dimsopt  */
#line 2324 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5237 "cmDependsJavaParser.cxx"
    break;

  case 258: /* ArrayCreationExpression: New PrimitiveType Dims ArrayInitializer  */
#line 2333 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5249 "cmDependsJavaParser.cxx"
    break;

  case 259: /* ArrayCreationExpression: New ClassOrInterfaceType Dims ArrayInitializer  */
#line 2342 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5261 "cmDependsJavaParser.cxx"
    break;

  case 260: /* Dimsopt: %empty  */
#line 2351 "cmDependsJavaParser.y"
{
  jpElementStart(0);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5272 "cmDependsJavaParser.cxx"
    break;

  case 261: /* Dimsopt: Dims  */
#line 2359 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5284 "cmDependsJavaParser.cxx"
    break;

  case 262: /* DimExprs: DimExpr  */
#line 2369 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5296 "cmDependsJavaParser.cxx"
    break;

  case 263: /* DimExprs: DimExprs DimExpr  */
#line 2378 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5308 "cmDependsJavaParser.cxx"
    break;

  case 264: /* DimExpr: jp_BRACKETSTART Expression jp_BRACKETEND  */
#line 2388 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5320 "cmDependsJavaParser.cxx"
    break;

  case 265: /* Dims: jp_BRACKETSTART jp_BRACKETEND  */
#line 2398 "cmDependsJavaParser.y"
{
  jpElementStart(2);

}
#line 5329 "cmDependsJavaParser.cxx"
    break;

  case 266: /* Dims: Dims jp_BRACKETSTART jp_BRACKETEND  */
#line 2404 "cmDependsJavaParser.y"
{
  jpElementStart(3);

}
#line 5338 "cmDependsJavaParser.cxx"
    break;

  case 267: /* FieldAccess: Primary jp_DOT Identifier  */
#line 2411 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5351 "cmDependsJavaParser.cxx"
    break;

  case 268: /* FieldAccess: jp_SUPER jp_DOT Identifier  */
#line 2421 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5364 "cmDependsJavaParser.cxx"
    break;

  case 269: /* FieldAccess: jp_THIS jp_DOT Identifier  */
#line 2431 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5377 "cmDependsJavaParser.cxx"
    break;

  case 270: /* FieldAccess: Primary jp_DOT jp_THIS  */
#line 2441 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5390 "cmDependsJavaParser.cxx"
    break;

  case 271: /* MethodInvocation: Name jp_PARESTART ArgumentListopt jp_PAREEND  */
#line 2452 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5403 "cmDependsJavaParser.cxx"
    break;

  case 272: /* MethodInvocation: Primary jp_DOT Identifier jp_PARESTART ArgumentListopt jp_PAREEND  */
#line 2462 "cmDependsJavaParser.y"
{
  jpElementStart(6);
  yyGetParser->DeallocateParserType(&((yyvsp[-5].str)));
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  jpCheckEmpty(6);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5417 "cmDependsJavaParser.cxx"
    break;

  case 273: /* MethodInvocation: jp_SUPER jp_DOT Identifier jp_PARESTART ArgumentListopt jp_PAREEND  */
#line 2473 "cmDependsJavaParser.y"
{
  jpElementStart(6);
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  jpCheckEmpty(6);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5430 "cmDependsJavaParser.cxx"
    break;

  case 274: /* MethodInvocation: jp_THIS jp_DOT Identifier jp_PARESTART ArgumentListopt jp_PAREEND  */
#line 2483 "cmDependsJavaParser.y"
{
  jpElementStart(6);
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  jpCheckEmpty(6);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5443 "cmDependsJavaParser.cxx"
    break;

  case 275: /* ArrayAccess: Name jp_BRACKETSTART Expression jp_BRACKETEND  */
#line 2494 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  yyGetParser->DeallocateParserType(&((yyvsp[-3].str)));
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5456 "cmDependsJavaParser.cxx"
    break;

  case 276: /* ArrayAccess: PrimaryNoNewArray jp_BRACKETSTART Expression jp_BRACKETEND  */
#line 2504 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5468 "cmDependsJavaParser.cxx"
    break;

  case 277: /* PostfixExpression: Primary  */
#line 2514 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5480 "cmDependsJavaParser.cxx"
    break;

  case 278: /* PostfixExpression: Name  */
#line 2523 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5492 "cmDependsJavaParser.cxx"
    break;

  case 279: /* PostfixExpression: ArrayType jp_DOT jp_CLASS  */
#line 2532 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5504 "cmDependsJavaParser.cxx"
    break;

  case 280: /* PostfixExpression: PostIncrementExpression  */
#line 2541 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5516 "cmDependsJavaParser.cxx"
    break;

  case 281: /* PostfixExpression: PostDecrementExpression  */
#line 2550 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5528 "cmDependsJavaParser.cxx"
    break;

  case 282: /* PostIncrementExpression: PostfixExpression jp_PLUSPLUS  */
#line 2560 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5540 "cmDependsJavaParser.cxx"
    break;

  case 283: /* PostDecrementExpression: PostfixExpression jp_MINUSMINUS  */
#line 2570 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5552 "cmDependsJavaParser.cxx"
    break;

  case 284: /* UnaryExpression: PreIncrementExpression  */
#line 2580 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5564 "cmDependsJavaParser.cxx"
    break;

  case 285: /* UnaryExpression: PreDecrementExpression  */
#line 2589 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5576 "cmDependsJavaParser.cxx"
    break;

  case 286: /* UnaryExpression: jp_PLUS UnaryExpression  */
#line 2598 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5588 "cmDependsJavaParser.cxx"
    break;

  case 287: /* UnaryExpression: jp_MINUS UnaryExpression  */
#line 2607 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5600 "cmDependsJavaParser.cxx"
    break;

  case 288: /* UnaryExpression: UnaryExpressionNotPlusMinus  */
#line 2616 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5612 "cmDependsJavaParser.cxx"
    break;

  case 289: /* PreIncrementExpression: jp_PLUSPLUS UnaryExpression  */
#line 2626 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5624 "cmDependsJavaParser.cxx"
    break;

  case 290: /* PreDecrementExpression: jp_MINUSMINUS UnaryExpression  */
#line 2636 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5636 "cmDependsJavaParser.cxx"
    break;

  case 291: /* UnaryExpressionNotPlusMinus: PostfixExpression  */
#line 2646 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5648 "cmDependsJavaParser.cxx"
    break;

  case 292: /* UnaryExpressionNotPlusMinus: jp_TILDE UnaryExpression  */
#line 2655 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5660 "cmDependsJavaParser.cxx"
    break;

  case 293: /* UnaryExpressionNotPlusMinus: jp_EXCLAMATION UnaryExpression  */
#line 2664 "cmDependsJavaParser.y"
{
  jpElementStart(2);
  jpCheckEmpty(2);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5672 "cmDependsJavaParser.cxx"
    break;

  case 294: /* UnaryExpressionNotPlusMinus: CastExpression  */
#line 2673 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5684 "cmDependsJavaParser.cxx"
    break;

  case 295: /* CastExpression: jp_PARESTART PrimitiveType Dimsopt jp_PAREEND UnaryExpression  */
#line 2683 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5696 "cmDependsJavaParser.cxx"
    break;

  case 296: /* CastExpression: jp_PARESTART Expression jp_PAREEND UnaryExpressionNotPlusMinus  */
#line 2692 "cmDependsJavaParser.y"
{
  jpElementStart(4);
  jpCheckEmpty(4);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5708 "cmDependsJavaParser.cxx"
    break;

  case 297: /* CastExpression: jp_PARESTART Name Dims jp_PAREEND UnaryExpressionNotPlusMinus  */
#line 2701 "cmDependsJavaParser.y"
{
  jpElementStart(5);

}
#line 5717 "cmDependsJavaParser.cxx"
    break;

  case 298: /* MultiplicativeExpression: UnaryExpression  */
#line 2708 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5729 "cmDependsJavaParser.cxx"
    break;

  case 299: /* MultiplicativeExpression: MultiplicativeExpression jp_TIMES UnaryExpression  */
#line 2717 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5741 "cmDependsJavaParser.cxx"
    break;

  case 300: /* MultiplicativeExpression: MultiplicativeExpression jp_DIVIDE UnaryExpression  */
#line 2726 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5753 "cmDependsJavaParser.cxx"
    break;

  case 301: /* MultiplicativeExpression: MultiplicativeExpression jp_PERCENT UnaryExpression  */
#line 2735 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5765 "cmDependsJavaParser.cxx"
    break;

  case 302: /* AdditiveExpression: MultiplicativeExpression  */
#line 2745 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5777 "cmDependsJavaParser.cxx"
    break;

  case 303: /* AdditiveExpression: AdditiveExpression jp_PLUS MultiplicativeExpression  */
#line 2754 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5789 "cmDependsJavaParser.cxx"
    break;

  case 304: /* AdditiveExpression: AdditiveExpression jp_MINUS MultiplicativeExpression  */
#line 2763 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5801 "cmDependsJavaParser.cxx"
    break;

  case 305: /* ShiftExpression: AdditiveExpression  */
#line 2773 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5813 "cmDependsJavaParser.cxx"
    break;

  case 306: /* ShiftExpression: ShiftExpression jp_LTLT AdditiveExpression  */
#line 2782 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5825 "cmDependsJavaParser.cxx"
    break;

  case 307: /* ShiftExpression: ShiftExpression jp_GTGT AdditiveExpression  */
#line 2791 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5837 "cmDependsJavaParser.cxx"
    break;

  case 308: /* ShiftExpression: ShiftExpression jp_GTGTGT AdditiveExpression  */
#line 2800 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5849 "cmDependsJavaParser.cxx"
    break;

  case 309: /* RelationalExpression: ShiftExpression  */
#line 2810 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5861 "cmDependsJavaParser.cxx"
    break;

  case 310: /* RelationalExpression: RelationalExpression jp_LESSTHAN ShiftExpression  */
#line 2819 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5873 "cmDependsJavaParser.cxx"
    break;

  case 311: /* RelationalExpression: RelationalExpression jp_GREATER ShiftExpression  */
#line 2828 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5885 "cmDependsJavaParser.cxx"
    break;

  case 312: /* RelationalExpression: RelationalExpression jp_LTEQUALS ShiftExpression  */
#line 2837 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5897 "cmDependsJavaParser.cxx"
    break;

  case 313: /* RelationalExpression: RelationalExpression jp_GTEQUALS ShiftExpression  */
#line 2846 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5909 "cmDependsJavaParser.cxx"
    break;

  case 314: /* RelationalExpression: RelationalExpression jp_INSTANCEOF ReferenceType  */
#line 2855 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5921 "cmDependsJavaParser.cxx"
    break;

  case 315: /* EqualityExpression: RelationalExpression  */
#line 2865 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5933 "cmDependsJavaParser.cxx"
    break;

  case 316: /* EqualityExpression: EqualityExpression jp_EQUALSEQUALS RelationalExpression  */
#line 2874 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5945 "cmDependsJavaParser.cxx"
    break;

  case 317: /* EqualityExpression: EqualityExpression jp_EXCLAMATIONEQUALS RelationalExpression  */
#line 2883 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5957 "cmDependsJavaParser.cxx"
    break;

  case 318: /* AndExpression: EqualityExpression  */
#line 2893 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5969 "cmDependsJavaParser.cxx"
    break;

  case 319: /* AndExpression: AndExpression jp_AND EqualityExpression  */
#line 2902 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5981 "cmDependsJavaParser.cxx"
    break;

  case 320: /* ExclusiveOrExpression: AndExpression  */
#line 2912 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 5993 "cmDependsJavaParser.cxx"
    break;

  case 321: /* ExclusiveOrExpression: ExclusiveOrExpression jp_CARROT AndExpression  */
#line 2921 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6005 "cmDependsJavaParser.cxx"
    break;

  case 322: /* InclusiveOrExpression: ExclusiveOrExpression  */
#line 2931 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6017 "cmDependsJavaParser.cxx"
    break;

  case 323: /* InclusiveOrExpression: InclusiveOrExpression jp_PIPE ExclusiveOrExpression  */
#line 2940 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6029 "cmDependsJavaParser.cxx"
    break;

  case 324: /* ConditionalAndExpression: InclusiveOrExpression  */
#line 2950 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6041 "cmDependsJavaParser.cxx"
    break;

  case 325: /* ConditionalAndExpression: ConditionalAndExpression jp_ANDAND InclusiveOrExpression  */
#line 2959 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6053 "cmDependsJavaParser.cxx"
    break;

  case 326: /* ConditionalOrExpression: ConditionalAndExpression  */
#line 2969 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6065 "cmDependsJavaParser.cxx"
    break;

  case 327: /* ConditionalOrExpression: ConditionalOrExpression jp_PIPEPIPE ConditionalAndExpression  */
#line 2978 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6077 "cmDependsJavaParser.cxx"
    break;

  case 328: /* ConditionalExpression: ConditionalOrExpression  */
#line 2988 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6089 "cmDependsJavaParser.cxx"
    break;

  case 329: /* ConditionalExpression: ConditionalOrExpression jp_QUESTION Expression jp_COLON ConditionalExpression  */
#line 2997 "cmDependsJavaParser.y"
{
  jpElementStart(5);
  jpCheckEmpty(5);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6101 "cmDependsJavaParser.cxx"
    break;

  case 330: /* AssignmentExpression: ConditionalExpression  */
#line 3007 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6113 "cmDependsJavaParser.cxx"
    break;

  case 331: /* AssignmentExpression: Assignment  */
#line 3016 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6125 "cmDependsJavaParser.cxx"
    break;

  case 332: /* Assignment: LeftHandSide AssignmentOperator AssignmentExpression  */
#line 3026 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6137 "cmDependsJavaParser.cxx"
    break;

  case 333: /* LeftHandSide: Name  */
#line 3036 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  yyGetParser->DeallocateParserType(&((yyvsp[0].str)));
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6150 "cmDependsJavaParser.cxx"
    break;

  case 334: /* LeftHandSide: FieldAccess  */
#line 3046 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6162 "cmDependsJavaParser.cxx"
    break;

  case 335: /* LeftHandSide: ArrayAccess  */
#line 3055 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6174 "cmDependsJavaParser.cxx"
    break;

  case 336: /* AssignmentOperator: jp_EQUALS  */
#line 3065 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6186 "cmDependsJavaParser.cxx"
    break;

  case 337: /* AssignmentOperator: jp_TIMESEQUALS  */
#line 3074 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6198 "cmDependsJavaParser.cxx"
    break;

  case 338: /* AssignmentOperator: jp_DIVIDEEQUALS  */
#line 3083 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6210 "cmDependsJavaParser.cxx"
    break;

  case 339: /* AssignmentOperator: jp_PERCENTEQUALS  */
#line 3092 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6222 "cmDependsJavaParser.cxx"
    break;

  case 340: /* AssignmentOperator: jp_PLUSEQUALS  */
#line 3101 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6234 "cmDependsJavaParser.cxx"
    break;

  case 341: /* AssignmentOperator: jp_MINUSEQUALS  */
#line 3110 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6246 "cmDependsJavaParser.cxx"
    break;

  case 342: /* AssignmentOperator: jp_LESLESEQUALS  */
#line 3119 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6258 "cmDependsJavaParser.cxx"
    break;

  case 343: /* AssignmentOperator: jp_GTGTEQUALS  */
#line 3128 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6270 "cmDependsJavaParser.cxx"
    break;

  case 344: /* AssignmentOperator: jp_GTGTGTEQUALS  */
#line 3137 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6282 "cmDependsJavaParser.cxx"
    break;

  case 345: /* AssignmentOperator: jp_ANDEQUALS  */
#line 3146 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6294 "cmDependsJavaParser.cxx"
    break;

  case 346: /* AssignmentOperator: jp_CARROTEQUALS  */
#line 3155 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6306 "cmDependsJavaParser.cxx"
    break;

  case 347: /* AssignmentOperator: jp_PIPEEQUALS  */
#line 3164 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6318 "cmDependsJavaParser.cxx"
    break;

  case 348: /* Expression: AssignmentExpression  */
#line 3174 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6330 "cmDependsJavaParser.cxx"
    break;

  case 349: /* ConstantExpression: Expression  */
#line 3184 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6342 "cmDependsJavaParser.cxx"
    break;

  case 350: /* New: jp_NEW  */
#line 3194 "cmDependsJavaParser.y"
{
  jpElementStart(1);
  jpCheckEmpty(1);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6354 "cmDependsJavaParser.cxx"
    break;

  case 351: /* New: Name jp_DOT jp_NEW  */
#line 3203 "cmDependsJavaParser.y"
{
  jpElementStart(3);
  jpStoreClass((yyvsp[-2].str));
  jpCheckEmpty(3);
  (yyval.str) = 0;
  yyGetParser->SetCurrentCombine("");

}
#line 6367 "cmDependsJavaParser.cxx"
    break;


#line 6371 "cmDependsJavaParser.cxx"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yyscanner, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, yyscanner);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yyscanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (yyscanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, yyscanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yyscanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 3212 "cmDependsJavaParser.y"

/* End of grammar */

/*--------------------------------------------------------------------------*/
void cmDependsJava_yyerror(yyscan_t yyscanner, const char* message)
{
  yyGetParser->Error(message);
}
