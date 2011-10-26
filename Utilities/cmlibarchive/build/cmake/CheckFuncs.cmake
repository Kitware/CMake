# Check if the system has the specified function; treat glibc "stub"
# functions as nonexistent:
# CHECK_FUNCTION_EXISTS_GLIBC (FUNCTION FUNCVAR)
#
#  FUNCTION - the function(s) where the prototype should be declared
#  FUNCVAR - variable to define if the function does exist
#
# In particular, this understands the glibc convention of
# defining macros __stub_XXXX or __stub___XXXX if the function
# does appear in the library but is merely a stub that does nothing.
# By detecting this case, we can select alternate behavior on
# platforms that don't support this functionality.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
# Copyright (c) 2009, Michihiro NAKAJIMA
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(CheckFunctionExists)

macro(CHECK_FUNCTION_EXISTS_GLIBC _FUNC _FUNCVAR)
  if(NOT DEFINED ${_FUNCVAR})
   set(CHECK_STUB_FUNC_1 "__stub_${_FUNC}")
   set(CHECK_STUB_FUNC_2 "__stub___${_FUNC}")
   configure_file( ${libarchive_SOURCE_DIR}/build/cmake/CheckFuncs_stub.c.in
     ${CMAKE_BINARY_DIR}/cmake.tmp/CheckFuncs_stub.c IMMEDIATE)
   try_compile(__stub
     ${CMAKE_BINARY_DIR}
     ${CMAKE_BINARY_DIR}/cmake.tmp/CheckFuncs_stub.c
     COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
     CMAKE_FLAGS
     -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_INCLUDE_FILE_FLAGS}
     "${CHECK_INCLUDE_FILE_C_INCLUDE_DIRS}")
   if(__stub)
     set("${_FUNCVAR}" "" CACHE INTERNAL "Have function ${_FUNC}")
   else(__stub)
     CHECK_FUNCTION_EXISTS("${_FUNC}" "${_FUNCVAR}")
   endif(__stub)
   endif(NOT DEFINED ${_FUNCVAR})
endmacro(CHECK_FUNCTION_EXISTS_GLIBC)

