%{
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

/* IWYU pragma: no_forward_declare yyguts_t */

#ifndef __clang_analyzer__ /* Suppress clang-analyzer warnings */

#include <cmGccDepfileLexerHelper.h>
#include <string>
%}

%option prefix="cmGccDepfile_yy"
%option noyywrap
%option reentrant
%pointer

WSPACE [ \t]
NEWLINE \r?\n

%%
\${2}                  {
                         // Unescape the dollar sign.
                         yyextra->addToCurrentPath("$");
                       }
\\#                    {
                         // Unescape the hash.
                         yyextra->addToCurrentPath("#");
                       }
(\\\\)*\\[ ]           {
                         // 2N+1 backslashes plus space -> N backslashes plus space.
                         size_t c = (strlen(yytext) - 1) / 2;
                         std::string s(c, '\\');
                         s.push_back(' ');
                         yyextra->addToCurrentPath(s.c_str());
                       }
(\\\\)+[ ]             {
                         // 2N backslashes plus space -> 2N backslashes, end of filename.
                         yytext[strlen(yytext) - 1] = 0;
                         yyextra->addToCurrentPath(yytext);
                         yyextra->newDependency();
                       }
{WSPACE}*\\{NEWLINE}   {
                         // A line continuation ends the current file name.
                         yyextra->newRuleOrDependency();
                       }
{NEWLINE}              {
                         // A newline ends the current file name and the current rule.
                         yyextra->newEntry();
                       }
:{NEWLINE}             {
                         // A colon ends the rules
                         yyextra->newDependency();
                         // A newline after colon terminates current rule.
                         yyextra->newEntry();
                       }
:({WSPACE}+|\\{NEWLINE}) {
                         // A colon followed by space or line continuation ends the rules
                         // and starts a new dependency.
                         yyextra->newDependency();
                       }
{WSPACE}+              {
                         // Rules and dependencies are separated by blocks of whitespace.
                         yyextra->newRuleOrDependency();
                       }
[a-zA-Z0-9+,/_.~()}{%=@\x5B\x5D!\x80-\xFF-]+ {
                         // Got a span of plain text.
                         yyextra->addToCurrentPath(yytext);
                       }
.                      {
                         // Got an otherwise unmatched character.
                         yyextra->addToCurrentPath(yytext);
                       }

%%

/*--------------------------------------------------------------------------*/

#endif /* __clang_analyzer__ */
