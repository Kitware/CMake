# - Find Ruby
# This module finds if Ruby is installed and determines where the include files
# and libraries are. It also determines what the name of the library is. This
# code sets the following variables:
#
#  RUBY_INCLUDE_PATH = path to where ruby.h can be found
#  RUBY_EXECUTABLE   = full path to the ruby binary

# Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
# See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.


if(RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
   # Already in cache, be silent
   set(RUBY_FIND_QUIETLY TRUE)
endif (RUBY_LIBRARY AND RUBY_INCLUDE_PATH)

#   RUBY_ARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"archdir"@:>@)'`
#   RUBY_SITEARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitearchdir"@:>@)'`
#   RUBY_SITEDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitelibdir"@:>@)'`
#   RUBY_LIBDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"libdir"@:>@)'`
#   RUBY_LIBRUBYARG=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"LIBRUBYARG_SHARED"@:>@)'`

FIND_PROGRAM(RUBY_EXECUTABLE NAMES ruby ruby1.8 ruby18 )

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "puts Config::CONFIG['archdir']"
   OUTPUT_VARIABLE RUBY_ARCH_DIR)


EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "puts Config::CONFIG['libdir']"
   OUTPUT_VARIABLE RUBY_POSSIBLE_LIB_PATH)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "puts Config::CONFIG['rubylibdir']"
   OUTPUT_VARIABLE RUBY_RUBY_LIB_PATH)

# remove the new lines from the output by replacing them with empty strings
STRING(REPLACE "\n" "" RUBY_ARCH_DIR "${RUBY_ARCH_DIR}")
STRING(REPLACE "\n" "" RUBY_POSSIBLE_LIB_PATH "${RUBY_POSSIBLE_LIB_PATH}")
STRING(REPLACE "\n" "" RUBY_RUBY_LIB_PATH "${RUBY_RUBY_LIB_PATH}")


FIND_PATH(RUBY_INCLUDE_PATH
   NAMES ruby.h
  PATHS
   ${RUBY_ARCH_DIR}
  /usr/lib/ruby/1.8/i586-linux-gnu/ )

FIND_LIBRARY(RUBY_LIBRARY
  NAMES ruby ruby1.8
  PATHS ${RUBY_POSSIBLE_LIB_PATH}
  )

MARK_AS_ADVANCED(
  RUBY_EXECUTABLE
  RUBY_LIBRARY
  RUBY_INCLUDE_PATH
  )
