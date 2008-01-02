# - Find Ruby
# This module finds if Ruby is installed and determines where the include files
# and libraries are. It also determines what the name of the library is. This
# code sets the following variables:
#
#  RUBY_INCLUDE_PATH = path to where ruby.h can be found
#  RUBY_EXECUTABLE   = full path to the ruby binary
#  RUBY_LIBRARY      = full path to the ruby library

# Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
# See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.


#   RUBY_ARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"archdir"@:>@)'`
#   RUBY_SITEARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitearchdir"@:>@)'`
#   RUBY_SITEDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"sitelibdir"@:>@)'`
#   RUBY_LIBDIR=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"libdir"@:>@)'`
#   RUBY_LIBRUBYARG=`$RUBY -r rbconfig -e 'printf("%s",Config::CONFIG@<:@"LIBRUBYARG_SHARED"@:>@)'`

FIND_PROGRAM(RUBY_EXECUTABLE NAMES ruby ruby1.8 ruby18 ruby1.9 ruby19)


IF(RUBY_EXECUTABLE  AND NOT  RUBY_ARCH_DIR)
   EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['archdir']"
      OUTPUT_VARIABLE RUBY_ARCH_DIR)

   EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['libdir']"
      OUTPUT_VARIABLE RUBY_POSSIBLE_LIB_DIR)

   EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['rubylibdir']"
      OUTPUT_VARIABLE RUBY_RUBY_LIB_DIR)

   # site_ruby
   EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['sitearchdir']"
      OUTPUT_VARIABLE RUBY_SITEARCH_DIR)

   EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['sitelibdir']"
      OUTPUT_VARIABLE RUBY_SITELIB_DIR)

   # vendor_ruby available ?
   EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r vendor-specific -e "print 'true'"
      OUTPUT_VARIABLE RUBY_HAS_VENDOR_RUBY  ERROR_QUIET)

   IF(RUBY_HAS_VENDOR_RUBY)
      EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['vendorlibdir']"
         OUTPUT_VARIABLE RUBY_VENDORLIB_DIR)

      EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['vendorarchdir']"
         OUTPUT_VARIABLE RUBY_VENDORARCH_DIR)
   ENDIF(RUBY_HAS_VENDOR_RUBY)

   # save the results in the cache so we don't have to run ruby the next time again
   SET(RUBY_ARCH_DIR         ${RUBY_ARCH_DIR}         CACHE PATH "The Ruby arch dir")
   SET(RUBY_POSSIBLE_LIB_DIR ${RUBY_POSSIBLE_LIB_DIR} CACHE PATH "The Ruby lib dir")
   SET(RUBY_RUBY_LIB_DIR     ${RUBY_RUBY_LIB_DIR}     CACHE PATH "The Ruby ruby-lib dir")
   SET(RUBY_SITEARCH_DIR     ${RUBY_SITEARCH_DIR}     CACHE PATH "The Ruby site arch dir")
   SET(RUBY_SITELIB_DIR      ${RUBY_SITELIB_DIR}      CACHE PATH "The Ruby site lib dir")
   SET(RUBY_HAS_VENDOR_RUBY  ${RUBY_HAS_VENDOR_RUBY}  CACHE BOOL "Vendor Ruby is available")
   SET(RUBY_VENDORARCH_DIR   ${RUBY_VENDORARCH_DIR}   CACHE PATH "The Ruby vendor arch dir")
   SET(RUBY_VENDORLIB_DIR    ${RUBY_VENDORLIB_DIR}    CACHE PATH "The Ruby vendor lib dir")

ENDIF(RUBY_EXECUTABLE  AND NOT  RUBY_ARCH_DIR)

# for compatibility
SET(RUBY_POSSIBLE_LIB_PATH ${RUBY_POSSIBLE_LIB_DIR})
SET(RUBY_RUBY_LIB_PATH ${RUBY_RUBY_LIB_DIR})


FIND_PATH(RUBY_INCLUDE_PATH
   NAMES ruby.h
   PATHS
   ${RUBY_ARCH_DIR}
  /usr/lib/ruby/1.8/i586-linux-gnu/ )

# search the ruby library, the version for MSVC can have the "msvc" prefix and the "static" suffix
FIND_LIBRARY(RUBY_LIBRARY
  NAMES ruby ruby1.8 ruby1.9
        msvcrt-ruby18 msvcrt-ruby19 msvcrt-ruby18-static msvcrt-ruby19-static
  PATHS ${RUBY_POSSIBLE_LIB_DIR}
  )

MARK_AS_ADVANCED(
  RUBY_EXECUTABLE
  RUBY_LIBRARY
  RUBY_INCLUDE_PATH
  RUBY_ARCH_DIR
  RUBY_POSSIBLE_LIB_DIR
  RUBY_RUBY_LIB_DIR
  RUBY_SITEARCH_DIR
  RUBY_SITELIB_DIR
  RUBY_HAS_VENDOR_RUBY
  RUBY_VENDORARCH_DIR
  RUBY_VENDORLIB_DIR
  )
