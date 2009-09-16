# a simple C only test case
cmake_minimum_required (VERSION 2.6)
PROJECT (FunctionTest)

FUNCTION(FAILED testname)
  MESSAGE(SEND_ERROR "${testname} failed ${ARGN}")
ENDFUNCTION(FAILED)

FUNCTION(PASS testname)
  MESSAGE("${testname} passed ${ARGN}")
ENDFUNCTION(PASS)


# test scope
SET(COUNT 3)
FUNCTION(scope_test)
  SET(COUNT 4)
ENDFUNCTION(scope_test)
scope_test()
IF(COUNT EQUAL "3")
  PASS("scope")
ELSE(COUNT EQUAL "3")
  FAILED("COUNT Got: ${COUNT}")
ENDIF(COUNT EQUAL "3")

# test ARGC
FUNCTION(weird_name)
  IF("${ARGC}" EQUAL "3")
    PASS("ARGC")
  ELSE("${ARGC}" EQUAL "3")
    FAILED("ARGC" "Got: ${ARGC}")
  ENDIF("${ARGC}" EQUAL "3")
ENDFUNCTION(weird_name)
WeIrD_nAmE(a1 a2 a3)

# test ARGN
FUNCTION(test_argn_function argument)
  IF("${ARGN}" EQUAL "3")
    PASS("ARGN")
  ELSE("${ARGN}" EQUAL "3")
    FAILED("ARGN" "Got: ${ARGN}")
  ENDIF("${ARGN}" EQUAL "3")
ENDFUNCTION(test_argn_function)
Test_Argn_Function(ignored 3)

# test argument naming and raise scope
function(track_find_variable cache_variable is_changed)
 set("${is_changed}" changed PARENT_SCOPE)
endfunction(track_find_variable)
track_find_variable(testvar is_changed)
if ("${is_changed}" STREQUAL changed)
  pass("same argument name test")
else ("${is_changed}" STREQUAL changed)
  pass("same argument name test")
endif ("${is_changed}" STREQUAL changed)

include("Util.cmake")
tester()
if (tester_res STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  pass("CMAKE_CURRENT_LIST_FILE test")
else (tester_res STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  pass("CMAKE_CURRENT_LIST_FILE test")
endif (tester_res STREQUAL "${CMAKE_CURRENT_LIST_FILE}")



# test recursion and return via set(... PARENT_SCOPE)
function (factorial argument result)
  if (argument LESS 2)
    set (lresult 1)
  else (argument LESS 2)
    math (EXPR temp "${argument} - 1")
    factorial (${temp} tresult)
    math (EXPR lresult "${argument}*${tresult}")
  endif (argument LESS 2)
  set ("${result}" "${lresult}" PARENT_SCOPE)
endfunction (factorial)

factorial (5 fresult)
if (fresult EQUAL 120)
  pass("factorial")
else (fresult EQUAL 120)
  failed ("factorial, computed ${fresult} instead of 120")
endif (fresult EQUAL 120)



# case test
FUNCTION(strange_function m)
  SET("${m}" strange_function PARENT_SCOPE)
ENDFUNCTION(strange_function m)

STRANGE_FUNCTION(var)
set(second_var "second_var")
IF("${var}" STREQUAL "strange_function" AND "${second_var}" STREQUAL "second_var")
  PASS("Case Test" "(${var} ${second_var})")
ELSE("${var}" STREQUAL "strange_function" AND "${second_var}" STREQUAL "second_var")
  FAILED("Case test" "(${var} ${second_var})")
ENDIF("${var}" STREQUAL "strange_function" AND "${second_var}" STREQUAL "second_var")

# test backing up command
FUNCTION(ADD_EXECUTABLE exec)
  _ADD_EXECUTABLE(mini${exec} ${ARGN})
ENDFUNCTION(ADD_EXECUTABLE)

# var undef case
FUNCTION(undef_var m)
  SET("${m}" PARENT_SCOPE)
ENDFUNCTION(undef_var)

SET(FUNCTION_UNDEFINED 1)
undef_var(FUNCTION_UNDEFINED)
IF(DEFINED FUNCTION_UNDEFINED)
  FAILED("Function Undefine Test" "(${FUNCTION_UNDEFINED})")
ELSE(DEFINED FUNCTION_UNDEFINED)
  PASS("Function Undefine Test" "(${FUNCTION_UNDEFINED})")
ENDIF(DEFINED FUNCTION_UNDEFINED)

# Subdirectory scope raise.
SET(SUBDIR_UNDEFINED 1)
ADD_SUBDIRECTORY(SubDirScope)
IF(DEFINED SUBDIR_UNDEFINED)
  FAILED("Subdir Undefine Test" "(${SUBDIR_UNDEFINED})")
ELSE(DEFINED SUBDIR_UNDEFINED)
  PASS("Subdir Undefine Test" "(${SUBDIR_UNDEFINED})")
ENDIF(DEFINED SUBDIR_UNDEFINED)
IF(DEFINED SUBDIR_DEFINED)
  PASS("Subdir Define Test" "(${SUBDIR_DEFINED})")
ELSE(DEFINED SUBDIR_DEFINED)
  FAILED("Subdir Define Test" "(${SUBDIR_DEFINED})")
ENDIF(DEFINED SUBDIR_DEFINED)

# Test function-scoped directory.
FUNCTION(ADD_SUBDIR2 dir)
  ADD_SUBDIRECTORY("${dir}" "${dir}2")
  # The parent scope sets in the subdir should be visible here.
  IF(DEFINED SUBDIR_UNDEFINED)
    FAILED("Subdir Function Undefine Test 1" "(${SUBDIR_UNDEFINED})")
  ELSE(DEFINED SUBDIR_UNDEFINED)
    PASS("Subdir Function Undefine Test 1" "(${SUBDIR_UNDEFINED})")
  ENDIF(DEFINED SUBDIR_UNDEFINED)
  IF(DEFINED SUBDIR_DEFINED)
    PASS("Subdir Function Define Test 1" "(${SUBDIR_DEFINED})")
  ELSE(DEFINED SUBDIR_DEFINED)
    FAILED("Subdir Function Define Test 1" "(${SUBDIR_DEFINED})")
  ENDIF(DEFINED SUBDIR_DEFINED)
ENDFUNCTION(ADD_SUBDIR2)

# Reset test variables.
SET(SUBDIR_UNDEFINED 1)
SET(SUBDIR_DEFINED)

# Run test function.
ADD_SUBDIR2(SubDirScope)

# The parent scope sets in the subdir should not be visible here.
IF(DEFINED SUBDIR_UNDEFINED)
  PASS("Subdir Function Undefine Test 2" "(${SUBDIR_UNDEFINED})")
ELSE(DEFINED SUBDIR_UNDEFINED)
  FAILED("Subdir Function Undefine Test 2" "(${SUBDIR_UNDEFINED})")
ENDIF(DEFINED SUBDIR_UNDEFINED)
IF(DEFINED SUBDIR_DEFINED)
  FAILED("Subdir Function Define Test 2" "(${SUBDIR_DEFINED})")
ELSE(DEFINED SUBDIR_DEFINED)
  PASS("Subdir Function Define Test 2" "(${SUBDIR_DEFINED})")
ENDIF(DEFINED SUBDIR_DEFINED)

ADD_EXECUTABLE(FunctionTest functionTest.c)
