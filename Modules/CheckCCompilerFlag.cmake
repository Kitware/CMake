# - Check whether the C compiler supports a given flag.
# CHECK_C_COMPILER_FLAG(<flag> <var>)
#  <flag> - the compiler flag
#  <var>  - variable to store the result
# This internally calls the check_c_source_compiles macro.
# See help for CheckCSourceCompiles for a listing of variables
# that can modify the build.

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


INCLUDE(CheckCSourceCompiles)

MACRO (CHECK_C_COMPILER_FLAG _FLAG _RESULT)
   SET(SAFE_CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS}")
   SET(CMAKE_REQUIRED_DEFINITIONS "${_FLAG}")
   CHECK_C_SOURCE_COMPILES("int main() { return 0;}" ${_RESULT}
     # Some compilers do not fail with a bad flag
     FAIL_REGEX "unrecognized option"                       # GNU
     FAIL_REGEX "ignoring unknown option"                   # MSVC
     FAIL_REGEX "[Uu]nknown option"                         # HP
     )
   SET (CMAKE_REQUIRED_DEFINITIONS "${SAFE_CMAKE_REQUIRED_DEFINITIONS}")
ENDMACRO (CHECK_C_COMPILER_FLAG)

