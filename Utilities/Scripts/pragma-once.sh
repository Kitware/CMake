#!/bin/sh

files=$(git ls-files \
  'Source/*.h' \
  'Source/*.h.in' \
  'Source/*.hxx' \
  'Utilities/cm3p/*.h' \
  'Utilities/cmThirdParty.h.in' \
  'Utilities/std/cm' \
  'Utilities/std/cmext' \
  'Utilities/std/cmSTL.hxx.in' \
  | grep -v '^Source/CPack/cmCPackConfigure\.h\.in$' \
  | grep -v '^Source/cmCPluginAPI\.h$' \
  | grep -v '^Source/cmVersionConfig\.h\.in$' \
  | grep -v '^Source/CursesDialog/form/' \
  | grep -v '^Source/LexerParser/' \
  | grep -v '^Source/kwsys/' \
  | grep -v '\.cxx$' \
)

perl -i Utilities/Scripts/pragma-once.pl $files
