#!/usr/bin/env bash

set -e

pushd "${BASH_SOURCE%/*}/../../Source/LexerParser" > /dev/null

for lexer in            \
    CommandArgument     \
    DependsJava         \
    Expr                \
    Fortran
do
    echo "Generating Lexer ${lexer}"
    flex --nounistd -DFLEXINT_H --noline --header-file=cm${lexer}Lexer.h -ocm${lexer}Lexer.cxx cm${lexer}Lexer.in.l
    sed -i 's/\s*$//'                       cm${lexer}Lexer.h cm${lexer}Lexer.cxx   # remove trailing whitespaces
    sed -i '${/^$/d;}'                      cm${lexer}Lexer.h cm${lexer}Lexer.cxx   # remove blank line at the end
    sed -i '1i#include "cmStandardLexer.h"' cm${lexer}Lexer.cxx                     # add cmStandardLexer.h include
done


# these lexers (at the moment only the ListFileLexer) are compiled as C and do not generate a header
for lexer in ListFile
do

    echo "Generating Lexer ${lexer}"
    flex --nounistd -DFLEXINT_H --noline -ocm${lexer}Lexer.c cm${lexer}Lexer.in.l
    sed -i 's/\s*$//'                       cm${lexer}Lexer.c   # remove trailing whitespaces
    sed -i '${/^$/d;}'                      cm${lexer}Lexer.c   # remove blank line at the end
    sed -i '1i#include "cmStandardLexer.h"' cm${lexer}Lexer.c   # add cmStandardLexer.h include

done

popd > /dev/null
