%{
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/*

This file must be translated to C++ and modified to build everywhere.

Run flex >= 2.6 like this:

  flex --nounistd -DFLEXINT_H --noline --header-file=cmCTestProcessesLexer.h -ocmCTestProcessesLexer.cxx cmCTestProcessesLexer.in.l

Modify cmCTestProcessesLexer.cxx:
  - remove trailing whitespace:              sed -i 's/\s*$//' cmCTestProcessesLexer.h cmCTestProcessesLexer.cxx
  - remove blank lines at end of file:       sed -i '${/^$/d;}' cmCTestProcessesLexer.h cmCTestProcessesLexer.cxx
  - #include "cmStandardLexer.h" at the top: sed -i '1i#include "cmStandardLexer.h"' cmCTestProcessesLexer.cxx

*/

/* IWYU pragma: no_forward_declare yyguts_t */

#ifndef __clang_analyzer__ /* Suppress clang scan-build warnings */

#include "cmCTestProcessesLexerHelper.h"

#include <string>

#include <cstddef>

/*--------------------------------------------------------------------------*/
%}

%option prefix="cmCTestProcesses_yy"

%option reentrant
%option noyywrap
%option nodefault
%pointer

%s PROCESSES_START
%s PROCESSES_END
%s RESOURCE_START
%s RESOURCE_COUNT
%s RESOURCE_END

NUMBER [0-9]+
IDENTIFIER [a-z_][a-z0-9_]*

%%

<INITIAL,PROCESSES_START,RESOURCE_START>{IDENTIFIER}: {
  BEGIN(RESOURCE_COUNT);
  yyextra->SetResourceType(std::string(yytext, yyleng - 1));
}

<INITIAL,PROCESSES_START>{NUMBER} {
  BEGIN(PROCESSES_END);
  std::size_t len = yyleng;
  yyextra->SetProcessCount(std::stoll(yytext, &len, 10));
}

<RESOURCE_COUNT>{NUMBER} {
  BEGIN(RESOURCE_END);
  std::size_t len = yyleng;
  yyextra->SetNeededSlots(std::stoll(yytext, &len, 10));
  yyextra->WriteRequirement();
}

<PROCESSES_END,RESOURCE_END>,+ {
  BEGIN(RESOURCE_START);
}

<INITIAL,PROCESSES_START,RESOURCE_START>;+ {
  BEGIN(PROCESSES_START);
}

<PROCESSES_END,RESOURCE_END>;+ {
  BEGIN(PROCESSES_START);
  yyextra->WriteProcess();
}

<RESOURCE_START,PROCESSES_END,RESOURCE_END><<EOF>> {
  yyextra->WriteProcess();
  return 0;
}

<INITIAL,PROCESSES_START><<EOF>> {
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
