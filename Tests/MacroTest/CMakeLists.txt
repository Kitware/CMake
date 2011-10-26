# a simple C only test case
cmake_minimum_required (VERSION 2.6)
project(MacroTest)

macro(FAILED testname)
  message(SEND_ERROR "${testname} failed ${ARGN}")
endmacro(FAILED)

macro(PASS testname)
  message("${testname} passed ${ARGN}")
endmacro(PASS)

# test ARGC
macro(weird_name)
  if("${ARGC}" EQUAL "3")
    PASS("ARGC")
  else("${ARGC}" EQUAL "3")
    FAILED("ARGC" "Got: ${ARGC}")
  endif("${ARGC}" EQUAL "3")
endmacro(weird_name)
WeIrD_nAmE(a1 a2 a3)

# test ARGN
macro(test_argn_macro argument)
  if("${ARGN}" EQUAL "3")
    PASS("ARGN")
  else("${ARGN}" EQUAL "3")
    FAILED("ARGN" "Got: ${ARGN}")
  endif("${ARGN}" EQUAL "3")
endmacro(test_argn_macro)
Test_Argn_Macro(ignored 3)

# case test
macro(strange_macro m)
  set("${m}" strange_macro)
endmacro(strange_macro m)
STRANGE_MACRO(var)
set(second_var "second_var")
if("${var}" STREQUAL "strange_macro" AND "${second_var}" STREQUAL "second_var")
  PASS("Case Test" "(${var} ${second_var})")
else("${var}" STREQUAL "strange_macro" AND "${second_var}" STREQUAL "second_var")
  FAILED("Case test" "(${var} ${second_var})")
endif("${var}" STREQUAL "strange_macro" AND "${second_var}" STREQUAL "second_var")

# test backing up command
macro(ADD_EXECUTABLE exec)
  _ADD_EXECUTABLE("mini${exec}" ${ARGN})
endmacro(ADD_EXECUTABLE)

include(CheckCSourceCompiles)
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
if(SOME_CHECK)
  message("CheckCSourceCompiles works")
else(SOME_CHECK)
  message(FATAL_ERROR "CheckCSourceCompiles does not work")
endif(SOME_CHECK)

include(CheckCXXSourceCompiles)
Check_CXX_Source_Compiles(
"
#include <stdio.h>
int main(int ac, char*av[]){
  if(ac > 1000){return *av[0];}
  return 0;
}"
SOME_CHECK)
if(SOME_CHECK)
  message("CheckCXXSourceCompiles works")
else(SOME_CHECK)
  message(FATAL_ERROR "CheckCXXSourceCompiles does not work")
endif(SOME_CHECK)

add_executable(MacroTest macroTest.c)

macro(GET_CURRENT_FILE var)
  set(${var} ${CMAKE_CURRENT_LIST_FILE})
endmacro(GET_CURRENT_FILE)
include(context.cmake)
