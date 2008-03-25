# a simple C only test case
cmake_minimum_required (VERSION 2.6)
PROJECT (MacroTest)

SET(CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")

MACRO(FAILED testname)
  MESSAGE(SEND_ERROR "${testname} failed ${ARGN}")
ENDMACRO(FAILED)

MACRO(PASS testname)
  MESSAGE("${testname} passed ${ARGN}")
ENDMACRO(PASS)

# test ARGC
MACRO(weird_name)
  IF("${ARGC}" EQUAL "3")
    PASS("ARGC")
  ELSE("${ARGC}" EQUAL "3")
    FAILED("ARGC" "Got: ${ARGC}")
  ENDIF("${ARGC}" EQUAL "3")
ENDMACRO(weird_name)
WeIrD_nAmE(a1 a2 a3)

# test ARGN
MACRO(test_argn_macro argument)
  IF("${ARGN}" EQUAL "3")
    PASS("ARGN")
  ELSE("${ARGN}" EQUAL "3")
    FAILED("ARGN" "Got: ${ARGN}")
  ENDIF("${ARGN}" EQUAL "3")
ENDMACRO(test_argn_macro)
Test_Argn_Macro(ignored 3)

# case test
MACRO(strange_macro m)
  SET("${m}" strange_macro)
ENDMACRO(strange_macro m)
STRANGE_MACRO(var)
set(second_var "second_var")
IF("${var}" STREQUAL "strange_macro" AND "${second_var}" STREQUAL "second_var")
  PASS("Case Test" "(${var} ${second_var})")
ELSE("${var}" STREQUAL "strange_macro" AND "${second_var}" STREQUAL "second_var")
  FAILED("Case test" "(${var} ${second_var})")
ENDIF("${var}" STREQUAL "strange_macro" AND "${second_var}" STREQUAL "second_var")

# test backing up command
MACRO(ADD_EXECUTABLE exec)
  _ADD_EXECUTABLE("mini${exec}" ${ARGN})
ENDMACRO(ADD_EXECUTABLE)

INCLUDE(CheckCSourceCompiles)
Check_C_Source_Compiles(
"
#include <stdio.h>
#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
  if(ac > 1000){return *av[0];}
  return 0;
}"
SOME_CHECK)
IF(SOME_CHECK)
  MESSAGE("CheckCSourceCompiles works")
ELSE(SOME_CHECK)
  MESSAGE(FATAL_ERROR "CheckCSourceCompiles does not work")
ENDIF(SOME_CHECK)

INCLUDE(CheckCXXSourceCompiles)
Check_CXX_Source_Compiles(
"
#include <stdio.h>
int main(int ac, char*av[]){
  if(ac > 1000){return *av[0];}
  return 0;
}"
SOME_CHECK)
IF(SOME_CHECK)
  MESSAGE("CheckCXXSourceCompiles works")
ELSE(SOME_CHECK)
  MESSAGE(FATAL_ERROR "CheckCXXSourceCompiles does not work")
ENDIF(SOME_CHECK)

ADD_EXECUTABLE(MacroTest macroTest.c)
