%{
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
/*

This file must be translated to C and modified to build everywhere.

Run bison like this:

  bison --yacc --name-prefix=cmDependsJava_yy --defines=cmDependsJavaParserTokens.h -ocmDependsJavaParser.cxx cmDependsJavaParser.y

Modify cmDependsJavaParser.c:
  - remove TABs

*/

/* Configure the parser to use a lexer object.  */
#define YYPARSE_PARAM yyscanner
#define YYLEX_PARAM yyscanner
#define YYERROR_VERBOSE 1
#define cmDependsJava_yyerror(x) \
        cmDependsJavaError(yyscanner, x)
#define yyGetParser (cmDependsJava_yyget_extra(yyscanner))

/*-------------------------------------------------------------------------*/
#include "cmDependsJavaParserHelper.h" /* Interface to parser object.  */
#include "cmDependsJavaLexer.h"  /* Interface to lexer object.  */
#include "cmDependsJavaParserTokens.h" /* Need YYSTYPE for YY_DECL.  */

/* Forward declare the lexer entry point.  */
YY_DECL;

/* Internal utility functions.  */
static void cmDependsJavaError(yyscan_t yyscanner, const char* message);

#define YYDEBUG 1
#define YYMAXDEPTH 1000000


#define CHECKEMPTY(cnt) yyGetParser->CheckEmpty(__LINE__, cnt, yyvsp);
#define ELEMENTSTART(cnt) yyGetParser->PrepareElement(&yyval)
#define STORECLASS(str) yyGetParser->AddClassFound(str); yyGetParser->DeallocateParserType(&(str))
/* Disable some warnings in the generated code.  */
#ifdef __BORLANDC__
# pragma warn -8004 /* Variable assigned a value that is not used.  */
#endif
#ifdef _MSC_VER
# pragma warning (disable: 4102) /* Unused goto label.  */
# pragma warning (disable: 4065) /* Switch statement contains default but no case. */
#endif
%}

/* Generate a reentrant parser object.  */
%pure_parser

/*
%union {
  char* string;
}
*/

/*-------------------------------------------------------------------------*/
/* Tokens */
%token ABSTRACT
%token ASSERT
%token BOOLEAN_TYPE
%token BREAK
%token BYTE_TYPE
%token CASE
%token CATCH
%token CHAR_TYPE
%token CLASS
%token CONTINUE
%token DEFAULT
%token DO
%token DOUBLE_TYPE
%token ELSE
%token EXTENDS
%token FINAL
%token FINALLY
%token FLOAT_TYPE
%token FOR
%token IF
%token IMPLEMENTS
%token IMPORT
%token INSTANCEOF
%token INT_TYPE
%token INTERFACE
%token LONG_TYPE
%token NATIVE
%token NEW
%token PACKAGE
%token PRIVATE
%token PROTECTED
%token PUBLIC
%token RETURN
%token SHORT_TYPE
%token STATIC
%token STRICTFP
%token SUPER
%token SWITCH
%token SYNCHRONIZED
%token THIS
%token THROW
%token THROWS
%token TRANSIENT
%token TRY
%token VOID
%token VOLATILE
%token WHILE

%token BOOLEANLITERAL
%token CHARACTERLITERAL
%token DECIMALINTEGERLITERAL
%token FLOATINGPOINTLITERAL
%token HEXINTEGERLITERAL
%token NULLLITERAL
%token STRINGLITERAL

%token NAME

%token AND
%token ANDAND
%token ANDEQUALS
%token BRACKETEND
%token BRACKETSTART
%token CARROT
%token CARROTEQUALS
%token COLON
%token COMMA
%token CURLYEND
%token CURLYSTART
%token DIVIDE
%token DIVIDEEQUALS
%token DOLLAR
%token DOT
%token EQUALS
%token EQUALSEQUALS
%token EXCLAMATION
%token EXCLAMATIONEQUALS
%token GREATER
%token GTEQUALS
%token GTGT
%token GTGTEQUALS
%token GTGTGT
%token GTGTGTEQUALS
%token LESLESEQUALS
%token LESSTHAN
%token LTEQUALS
%token LTLT
%token MINUS
%token MINUSEQUALS
%token MINUSMINUS
%token PAREEND
%token PARESTART
%token PERCENT
%token PERCENTEQUALS
%token PIPE
%token PIPEEQUALS
%token PIPEPIPE
%token PLUS
%token PLUSEQUALS
%token PLUSPLUS
%token QUESTION
%token SEMICOL
%token TILDE
%token TIMES
%token TIMESEQUALS

%token ERROR

/*-------------------------------------------------------------------------*/
/* grammar */
%%

Goal:
CompilationUnit
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

Literal:
IntegerLiteral
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
FLOATINGPOINTLITERAL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
BOOLEANLITERAL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
CHARACTERLITERAL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
STRINGLITERAL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
NULLLITERAL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

IntegerLiteral:
DECIMALINTEGERLITERAL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
HEXINTEGERLITERAL	
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

Type:
PrimitiveType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
ReferenceType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

PrimitiveType:
BYTE_TYPE
{
  ELEMENTSTART(0);
}
|
SHORT_TYPE
{
  ELEMENTSTART(0);
}
|
INT_TYPE
{
  ELEMENTSTART(0);
}
|
LONG_TYPE
{
  ELEMENTSTART(0);
}
|
CHAR_TYPE
{
  ELEMENTSTART(0);
}
|
FLOAT_TYPE
{
  ELEMENTSTART(0);
}
|
DOUBLE_TYPE
{
  ELEMENTSTART(0);
}
|
BOOLEAN_TYPE
{
  ELEMENTSTART(0);
}

ReferenceType:
ClassOrInterfaceType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
ArrayType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ClassOrInterfaceType:
Name
{
  ELEMENTSTART(1);
  STORECLASS($<str>1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ClassType:
ClassOrInterfaceType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

InterfaceType:
ClassOrInterfaceType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ArrayType:
PrimitiveType Dims
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
Name Dims
{
  ELEMENTSTART(2);
  STORECLASS($<str>1);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

Name:
SimpleName
{
  ELEMENTSTART(1);
  $<str>$ = $<str>1;
}
|
QualifiedName
{
  ELEMENTSTART(1);
  $<str>$ = $<str>1;
}

SimpleName:
Identifier
{
  ELEMENTSTART(1);
  $<str>$ = $<str>1;
}

Identifier:
NAME
{
  ELEMENTSTART(1);
  $<str>$ = $<str>1;
}
|
DOLLAR NAME
{
  ELEMENTSTART(2);
  $<str>$ = $<str>2;
}

QualifiedName:
Name DOT Identifier
{
  ELEMENTSTART(3);
  yyGetParser->AddClassFound($<str>1);
  yyGetParser->UpdateCombine($<str>1, $<str>3);
  yyGetParser->DeallocateParserType(&($<str>1));
  $<str>$ = const_cast<char*>(yyGetParser->GetCurrentCombine());
}
|
Name DOT CLASS
{
  ELEMENTSTART(3);
  STORECLASS($<str>1);
  CHECKEMPTY(3);
  yyGetParser->SetCurrentCombine("");
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
Name DOT THIS
{
  ELEMENTSTART(3);
  STORECLASS($<str>1);
  yyGetParser->SetCurrentCombine("");
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
SimpleType DOT CLASS
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

SimpleType:
PrimitiveType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
VOID
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

CompilationUnit:
PackageDeclarationopt ImportDeclarations TypeDeclarations
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

PackageDeclarationopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
PackageDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ImportDeclarations:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
ImportDeclarations ImportDeclaration
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

TypeDeclarations:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
TypeDeclarations TypeDeclaration
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

PackageDeclaration:
PACKAGE Name SEMICOL
{
  ELEMENTSTART(3);
  yyGetParser->SetCurrentPackage($<str>2);
  yyGetParser->DeallocateParserType(&($<str>2));
  yyGetParser->SetCurrentCombine("");
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ImportDeclaration:
SingleTypeImportDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
TypeImportOnDemandDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

SingleTypeImportDeclaration:
IMPORT Name SEMICOL
{
  ELEMENTSTART(3);
  yyGetParser->AddPackagesImport($<str>2);
  yyGetParser->DeallocateParserType(&($<str>2));
  yyGetParser->SetCurrentCombine("");
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

TypeImportOnDemandDeclaration:
IMPORT Name DOT TIMES SEMICOL
{
  ELEMENTSTART(5);
  std::string str = $<str>2;
  str += ".*";
  yyGetParser->AddPackagesImport(str.c_str());
  yyGetParser->DeallocateParserType(&($<str>2));
  yyGetParser->SetCurrentCombine("");
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

TypeDeclaration:
ClassDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
InterfaceDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
SEMICOL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

Modifiers:
Modifier
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
Modifiers Modifier
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

Modifier:
PUBLIC | PROTECTED | PRIVATE |
STATIC |
ABSTRACT | FINAL | NATIVE | SYNCHRONIZED | TRANSIENT | VOLATILE |
STRICTFP

ClassHeader:
Modifiersopt CLASS Identifier
{
  yyGetParser->StartClass($<str>3);
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(3);
}


ClassDeclaration:
ClassHeader ClassBody
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
|
ClassHeader Interfaces ClassBody
{
  ELEMENTSTART(3);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
|
ClassHeader Super ClassBody
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}
|
ClassHeader Super Interfaces ClassBody
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}

Modifiersopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
Modifiers
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

Super:
EXTENDS ClassType
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

Interfaces:
IMPLEMENTS InterfaceTypeList
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

InterfaceTypeList:
InterfaceType
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
InterfaceTypeList COMMA InterfaceType
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ClassBody:
CURLYSTART ClassBodyDeclarations CURLYEND
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ClassBodyDeclarations:
{
  ELEMENTSTART(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
ClassBodyDeclarations ClassBodyDeclaration
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ClassBodyDeclaration:
ClassMemberDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
StaticInitializer
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
ConstructorDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
TypeDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

ClassMemberDeclaration:
FieldDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
MethodDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

FieldDeclaration:
Modifiersopt Type VariableDeclarators SEMICOL
{
  ELEMENTSTART(4);
}

VariableDeclarators:
VariableDeclarator
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
VariableDeclarators COMMA VariableDeclarator
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

VariableDeclarator:
VariableDeclaratorId
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
VariableDeclaratorId EQUALS VariableInitializer
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

VariableDeclaratorId:
Identifier
{
  ELEMENTSTART(1);
  yyGetParser->DeallocateParserType(&($<str>1));
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
VariableDeclaratorId BRACKETSTART BRACKETEND
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

VariableInitializer:
Expression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
ArrayInitializer
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

MethodDeclaration:
MethodHeader SEMICOL
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
MethodHeader MethodBody
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
MethodHeader MethodBody SEMICOL
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}

MethodHeader:
Modifiersopt Type MethodDeclarator Throwsopt
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Modifiersopt VOID MethodDeclarator Throwsopt
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Throwsopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Throws
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

MethodDeclarator:
Identifier PARESTART FormalParameterListopt PAREEND
{
  ELEMENTSTART(4);
  yyGetParser->DeallocateParserType(&($<str>1));
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MethodDeclarator BRACKETSTART BRACKETEND
{
  ELEMENTSTART(3);

}

FormalParameterListopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
FormalParameterList

FormalParameterList:
FormalParameter
{
  ELEMENTSTART(1);

}
|
FormalParameterList COMMA FormalParameter
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

FormalParameter:
Modifiersopt Type VariableDeclaratorId
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Throws:
THROWS ClassTypeList
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ClassTypeList:
ClassType
{
  ELEMENTSTART(1);

}
|
ClassTypeList COMMA ClassType
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

MethodBody:
Block
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

StaticInitializer:
STATIC Block
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConstructorDeclaration:
Modifiersopt ConstructorDeclarator Throwsopt ConstructorBody
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Modifiersopt ConstructorDeclarator Throwsopt ConstructorBody SEMICOL
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConstructorDeclarator:
SimpleName PARESTART FormalParameterListopt PAREEND
{
  ELEMENTSTART(4);
  yyGetParser->DeallocateParserType(&($<str>1));
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConstructorBody:
CURLYSTART ExplicitConstructorInvocationopt BlockStatementsopt CURLYEND
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ExplicitConstructorInvocationopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ExplicitConstructorInvocationopt ExplicitConstructorInvocation
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ExplicitConstructorInvocation:
THIS PARESTART ArgumentListopt PAREEND SEMICOL
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SUPER PARESTART ArgumentListopt PAREEND SEMICOL
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

InterfaceHeader:
Modifiersopt INTERFACE Identifier
{
  yyGetParser->StartClass($<str>3);
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(3);
}

InterfaceDeclaration:
InterfaceHeader ExtendsInterfacesopt InterfaceBody
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
  yyGetParser->EndClass();
}

ExtendsInterfacesopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");
}
|
ExtendsInterfaces
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ExtendsInterfaces:
EXTENDS InterfaceType
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ExtendsInterfaces COMMA InterfaceType
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

InterfaceBody:
CURLYSTART InterfaceMemberDeclarations CURLYEND
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

InterfaceMemberDeclarations:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
InterfaceMemberDeclarations InterfaceMemberDeclaration
{
  ELEMENTSTART(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

InterfaceMemberDeclaration:
ConstantDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
AbstractMethodDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ClassDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ClassDeclaration SEMICOL
{
  ELEMENTSTART(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
InterfaceDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
InterfaceDeclaration SEMICOL
{
  ELEMENTSTART(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConstantDeclaration:
FieldDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

AbstractMethodDeclaration:
MethodHeader Semicols
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Semicols:
SEMICOL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Semicols SEMICOL
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ArrayInitializer:
CURLYSTART VariableInitializersOptional CURLYEND
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

VariableInitializersOptional:
{
  ELEMENTSTART(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
VariableInitializers
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
VariableInitializers COMMA
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

VariableInitializers:
VariableInitializer
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
VariableInitializers COMMA VariableInitializer
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Block:
CURLYSTART BlockStatementsopt CURLYEND
{
  ELEMENTSTART(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

BlockStatementsopt:
{
  ELEMENTSTART(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
BlockStatements
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

BlockStatements:
BlockStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
BlockStatements BlockStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

BlockStatement:
LocalVariableDeclarationStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Statement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ClassDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

LocalVariableDeclarationStatement:
LocalVariableDeclaration SEMICOL
{
  ELEMENTSTART(1);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

LocalVariableDeclaration:
Modifiers Type VariableDeclarators
{
  ELEMENTSTART(1);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Type VariableDeclarators
{
  ELEMENTSTART(1);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Statement:
StatementWithoutTrailingSubstatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
LabeledStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
IfThenStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
IfThenElseStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
WhileStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ForStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

StatementNoShortIf:
StatementWithoutTrailingSubstatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
LabeledStatementNoShortIf
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
IfThenElseStatementNoShortIf
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
WhileStatementNoShortIf
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ForStatementNoShortIf
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

StatementWithoutTrailingSubstatement:
Block
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
EmptyStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ExpressionStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SwitchStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
DoStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
BreakStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ContinueStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ReturnStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SynchronizedStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ThrowStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
TryStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
AssertStatement
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

EmptyStatement:
SEMICOL
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

LabeledStatement:
Identifier COLON Statement
{
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>1));
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

LabeledStatementNoShortIf:
Identifier COLON StatementNoShortIf
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ExpressionStatement:
StatementExpression SEMICOL
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

StatementExpression:
Assignment
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PreIncrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PreDecrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PostIncrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PostDecrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MethodInvocation
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ClassInstanceCreationExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

IfThenStatement:
IF PARESTART Expression PAREEND Statement
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

IfThenElseStatement:
IF PARESTART Expression PAREEND StatementNoShortIf ELSE Statement
{
  ELEMENTSTART(7);
  CHECKEMPTY(7);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

IfThenElseStatementNoShortIf:
IF PARESTART Expression PAREEND StatementNoShortIf ELSE StatementNoShortIf
{
  ELEMENTSTART(7);
  CHECKEMPTY(7);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

SwitchStatement:
SWITCH PARESTART Expression PAREEND SwitchBlock
{
  ELEMENTSTART(5);

}

SwitchBlock:
CURLYSTART SwitchBlockStatementGroups SwitchLabelsopt CURLYEND
{
  ELEMENTSTART(4);

}

SwitchLabelsopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SwitchLabels
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

SwitchBlockStatementGroups:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SwitchBlockStatementGroups SwitchBlockStatementGroup
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

SwitchBlockStatementGroup:
SwitchLabels BlockStatements
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

SwitchLabels:
SwitchLabel
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SwitchLabels SwitchLabel
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

SwitchLabel:
CASE ConstantExpression COLON
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
DEFAULT COLON
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

WhileStatement:
WHILE PARESTART Expression PAREEND Statement
{
  ELEMENTSTART(5);

}

WhileStatementNoShortIf:
WHILE PARESTART Expression PAREEND StatementNoShortIf
{
  ELEMENTSTART(5);

}

DoStatement:
DO Statement WHILE PARESTART Expression PAREEND SEMICOL
{
  ELEMENTSTART(7);

}

ForStatement:
FOR PARESTART ForInitopt SEMICOL Expressionopt SEMICOL ForUpdateopt PAREEND
Statement
{
  ELEMENTSTART(9);

}

ForUpdateopt:
{
  ELEMENTSTART(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ForUpdate
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ForInitopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ForInit
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ForStatementNoShortIf:
FOR PARESTART ForInitopt SEMICOL Expressionopt SEMICOL ForUpdateopt PAREEND
StatementNoShortIf
{
  ELEMENTSTART(9);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Expressionopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Expression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ForInit:
StatementExpressionList
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
LocalVariableDeclaration
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ForUpdate:
StatementExpressionList
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

StatementExpressionList:
StatementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
StatementExpressionList COMMA StatementExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

AssertStatement:
ASSERT Expression SEMICOL
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ASSERT Expression COLON Expression SEMICOL
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

BreakStatement:
BREAK Identifieropt SEMICOL
{
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>2));
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Identifieropt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Identifier
{
  ELEMENTSTART(1);

}

ContinueStatement:
CONTINUE Identifieropt SEMICOL
{
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>2));
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ReturnStatement:
RETURN Expressionopt SEMICOL
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ThrowStatement:
THROW Expression SEMICOL
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

SynchronizedStatement:
SYNCHRONIZED PARESTART Expression PAREEND Block
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

TryStatement:
TRY Block Catches
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
TRY Block Catchesopt Finally
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Catchesopt:
{
  ELEMENTSTART(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Catches
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Catches:
CatchClause
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Catches CatchClause
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

CatchClause:
CATCH PARESTART FormalParameter PAREEND Block
{
  ELEMENTSTART(5);

}

Finally:
FINALLY Block
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Primary:
PrimaryNoNewArray
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ArrayCreationExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

PrimaryNoNewArray:
Literal
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
THIS
{
  ELEMENTSTART(1);

}
|
PARESTART Expression PAREEND
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ClassInstanceCreationExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
FieldAccess
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MethodInvocation
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ArrayAccess
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ClassInstanceCreationExpression:
New ClassType PARESTART ArgumentListopt PAREEND ClassBodyOpt
{
  ELEMENTSTART(6);
  CHECKEMPTY(6);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ClassBodyOpt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ClassBody
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ArgumentListopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ArgumentList
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ArgumentList:
Expression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ArgumentList COMMA Expression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ArrayCreationExpression:
New PrimitiveType DimExprs Dimsopt
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
New ClassOrInterfaceType DimExprs Dimsopt
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
New PrimitiveType Dims ArrayInitializer
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
New ClassOrInterfaceType Dims ArrayInitializer
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Dimsopt:
{
  ELEMENTSTART(0);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Dims
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

DimExprs:
DimExpr
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
DimExprs DimExpr
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

DimExpr:
BRACKETSTART Expression BRACKETEND
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Dims:
BRACKETSTART BRACKETEND
{
  ELEMENTSTART(2);

}
|
Dims BRACKETSTART BRACKETEND
{
  ELEMENTSTART(3);

}

FieldAccess:
Primary DOT Identifier
{
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SUPER DOT Identifier
{
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
THIS DOT Identifier
{
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Primary DOT THIS
{
  ELEMENTSTART(3);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

MethodInvocation:
Name PARESTART ArgumentListopt PAREEND
{
  ELEMENTSTART(4);
  yyGetParser->DeallocateParserType(&($<str>1));
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Primary DOT Identifier PARESTART ArgumentListopt PAREEND
{
  ELEMENTSTART(6);
  yyGetParser->DeallocateParserType(&($<str>1));
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(6);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
SUPER DOT Identifier PARESTART ArgumentListopt PAREEND
{
  ELEMENTSTART(6);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(6);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
THIS DOT Identifier PARESTART ArgumentListopt PAREEND
{
  ELEMENTSTART(6);
  yyGetParser->DeallocateParserType(&($<str>3));
  CHECKEMPTY(6);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ArrayAccess:
Name BRACKETSTART Expression BRACKETEND
{
  ELEMENTSTART(4);
  yyGetParser->DeallocateParserType(&($<str>1));
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PrimaryNoNewArray BRACKETSTART Expression BRACKETEND
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

PostfixExpression:
Primary
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Name
{
  ELEMENTSTART(1);
  yyGetParser->DeallocateParserType(&($<str>1));
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ArrayType DOT CLASS
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PostIncrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PostDecrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

PostIncrementExpression:
PostfixExpression PLUSPLUS
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

PostDecrementExpression:
PostfixExpression MINUSMINUS
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

UnaryExpression:
PreIncrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PreDecrementExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PLUS UnaryExpression
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MINUS UnaryExpression
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
UnaryExpressionNotPlusMinus
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

PreIncrementExpression:
PLUSPLUS UnaryExpression
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

PreDecrementExpression:
MINUSMINUS UnaryExpression
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

UnaryExpressionNotPlusMinus:
PostfixExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
TILDE UnaryExpression
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
EXCLAMATION UnaryExpression
{
  ELEMENTSTART(2);
  CHECKEMPTY(2);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
CastExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

CastExpression:
PARESTART PrimitiveType Dimsopt PAREEND UnaryExpression
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PARESTART Expression PAREEND UnaryExpressionNotPlusMinus
{
  ELEMENTSTART(4);
  CHECKEMPTY(4);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PARESTART Name Dims PAREEND UnaryExpressionNotPlusMinus
{
  ELEMENTSTART(5);

}

MultiplicativeExpression:
UnaryExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MultiplicativeExpression TIMES UnaryExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MultiplicativeExpression DIVIDE UnaryExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MultiplicativeExpression PERCENT UnaryExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

AdditiveExpression:
MultiplicativeExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
AdditiveExpression PLUS MultiplicativeExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
AdditiveExpression MINUS MultiplicativeExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ShiftExpression:
AdditiveExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ShiftExpression LTLT AdditiveExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ShiftExpression GTGT AdditiveExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ShiftExpression GTGTGT AdditiveExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

RelationalExpression:
ShiftExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
RelationalExpression LESSTHAN ShiftExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
RelationalExpression GREATER ShiftExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
RelationalExpression LTEQUALS ShiftExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
RelationalExpression GTEQUALS ShiftExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
RelationalExpression INSTANCEOF ReferenceType
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

EqualityExpression:
RelationalExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
EqualityExpression EQUALSEQUALS RelationalExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
EqualityExpression EXCLAMATIONEQUALS RelationalExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

AndExpression:
EqualityExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
AndExpression AND EqualityExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ExclusiveOrExpression:
AndExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ExclusiveOrExpression CARROT AndExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

InclusiveOrExpression:
ExclusiveOrExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
InclusiveOrExpression PIPE ExclusiveOrExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConditionalAndExpression:
InclusiveOrExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ConditionalAndExpression ANDAND InclusiveOrExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConditionalOrExpression:
ConditionalAndExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ConditionalOrExpression PIPEPIPE ConditionalAndExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConditionalExpression:
ConditionalOrExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ConditionalOrExpression QUESTION Expression COLON ConditionalExpression
{
  ELEMENTSTART(5);
  CHECKEMPTY(5);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

AssignmentExpression:
ConditionalExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Assignment
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Assignment:
LeftHandSide AssignmentOperator AssignmentExpression
{
  ELEMENTSTART(3);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

LeftHandSide:
Name
{
  ELEMENTSTART(1);
  yyGetParser->DeallocateParserType(&($<str>1));
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
FieldAccess
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ArrayAccess
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

AssignmentOperator:
EQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
TIMESEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
DIVIDEEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PERCENTEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PLUSEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
MINUSEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
LESLESEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
GTGTEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
GTGTGTEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
ANDEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
CARROTEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
PIPEEQUALS
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

Expression:
AssignmentExpression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

ConstantExpression:
Expression
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

New:
NEW
{
  ELEMENTSTART(1);
  CHECKEMPTY(1);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}
|
Name DOT NEW
{
  ELEMENTSTART(3);
  STORECLASS($<str>1);
  CHECKEMPTY(3);
  $<str>$ = 0;
  yyGetParser->SetCurrentCombine("");

}

%%
/* End of grammar */

/*--------------------------------------------------------------------------*/
void cmDependsJavaError(yyscan_t yyscanner, const char* message)
{
  yyGetParser->Error(message);
}

