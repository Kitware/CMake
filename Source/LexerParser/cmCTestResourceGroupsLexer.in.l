%{
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/*

This file must be translated to C++ and modified to build everywhere.

Run flex >= 2.6 like this:

  flex --nounistd -DFLEXINT_H --noline --header-file=cmCTestResourceGroupsLexer.h -ocmCTestResourceGroupsLexer.cxx cmCTestResourceGroupsLexer.in.l

Modify cmCTestResourceGroupsLexer.cxx:
  - remove trailing whitespace:              sed -i 's/\s*$//' cmCTestResourceGroupsLexer.h cmCTestResourceGroupsLexer.cxx
  - remove blank lines at end of file:       sed -i '${/^$/d;}' cmCTestResourceGroupsLexer.h cmCTestResourceGroupsLexer.cxx
  - #include "cmStandardLexer.h" at the top: sed -i '1i#include "cmStandardLexer.h"' cmCTestResourceGroupsLexer.cxx

*/

/* IWYU pragma: no_forward_declare yyguts_t */

#ifndef __clang_analyzer__ /* Suppress clang scan-build warnings */

#include "cmCTestResourceGroupsLexerHelper.h"

#include <string>

#include <cstddef>

/*--------------------------------------------------------------------------*/
%}

%option prefix="cmCTestResourceGroups_yy"

%option reentrant
%option noyywrap
%option nodefault
%pointer

%s RESOURCE_GROUPS_START
%s RESOURCE_GROUPS_END
%s RESOURCE_START
%s RESOURCE_COUNT
%s RESOURCE_END

NUMBER [0-9]+
IDENTIFIER [a-z_][a-z0-9_]*

%%

<INITIAL,RESOURCE_GROUPS_START,RESOURCE_START>{IDENTIFIER}: {
  BEGIN(RESOURCE_COUNT);
  yyextra->SetResourceType(std::string(yytext, yyleng - 1));
}

<INITIAL,RESOURCE_GROUPS_START>{NUMBER} {
  BEGIN(RESOURCE_GROUPS_END);
  std::size_t len = yyleng;
  yyextra->SetProcessCount(std::stoll(yytext, &len, 10));
}

<RESOURCE_COUNT>{NUMBER} {
  BEGIN(RESOURCE_END);
  std::size_t len = yyleng;
  yyextra->SetNeededSlots(std::stoll(yytext, &len, 10));
  yyextra->WriteRequirement();
}

<RESOURCE_GROUPS_END,RESOURCE_END>,+ {
  BEGIN(RESOURCE_START);
}

<INITIAL,RESOURCE_GROUPS_START,RESOURCE_START>;+ {
  BEGIN(RESOURCE_GROUPS_START);
}

<RESOURCE_GROUPS_END,RESOURCE_END>;+ {
  BEGIN(RESOURCE_GROUPS_START);
  yyextra->WriteProcess();
}

<RESOURCE_START,RESOURCE_GROUPS_END,RESOURCE_END><<EOF>> {
  yyextra->WriteProcess();
  return 0;
}

<INITIAL,RESOURCE_GROUPS_START><<EOF>> {
  return 0;
}

<<EOF>> {
  return 1;
}

.|\n {
  return 1;
}

%%

/*--------------------------------------------------------------------------*/

#endif /* __clang_analyzer__ */
