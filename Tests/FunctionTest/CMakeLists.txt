# a simple C only test case
cmake_minimum_required (VERSION 2.6)
project(FunctionTest)

function(FAILED testname)
  message(SEND_ERROR "${testname} failed ${ARGN}")
endfunction(FAILED)

function(PASS testname)
  message("${testname} passed ${ARGN}")
endfunction(PASS)


# test scope
set(COUNT 3)
function(scope_test)
  set(COUNT 4)
endfunction(scope_test)
scope_test()
if(COUNT EQUAL "3")
  PASS("scope")
else(COUNT EQUAL "3")
  FAILED("COUNT Got: ${COUNT}")
endif(COUNT EQUAL "3")

# test ARGC
function(weird_name)
  if("${ARGC}" EQUAL "3")
    PASS("ARGC")
  else("${ARGC}" EQUAL "3")
    FAILED("ARGC" "Got: ${ARGC}")
  endif("${ARGC}" EQUAL "3")
endfunction(weird_name)
WeIrD_nAmE(a1 a2 a3)

# test ARGN
function(test_argn_function argument)
  if("${ARGN}" EQUAL "3")
    PASS("ARGN")
  else("${ARGN}" EQUAL "3")
    FAILED("ARGN" "Got: ${ARGN}")
  endif("${ARGN}" EQUAL "3")
endfunction(test_argn_function)
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
function(strange_function m)
  set("${m}" strange_function PARENT_SCOPE)
endfunction(strange_function m)

STRANGE_FUNCTION(var)
set(second_var "second_var")
if("${var}" STREQUAL "strange_function" AND "${second_var}" STREQUAL "second_var")
  PASS("Case Test" "(${var} ${second_var})")
else("${var}" STREQUAL "strange_function" AND "${second_var}" STREQUAL "second_var")
  FAILED("Case test" "(${var} ${second_var})")
endif("${var}" STREQUAL "strange_function" AND "${second_var}" STREQUAL "second_var")

# test backing up command
function(ADD_EXECUTABLE exec)
  _ADD_EXECUTABLE(mini${exec} ${ARGN})
endfunction(ADD_EXECUTABLE)

# var undef case
function(undef_var m)
  set("${m}" PARENT_SCOPE)
endfunction(undef_var)

set(FUNCTION_UNDEFINED 1)
undef_var(FUNCTION_UNDEFINED)
if(DEFINED FUNCTION_UNDEFINED)
  FAILED("Function Undefine Test" "(${FUNCTION_UNDEFINED})")
else(DEFINED FUNCTION_UNDEFINED)
  PASS("Function Undefine Test" "(${FUNCTION_UNDEFINED})")
endif(DEFINED FUNCTION_UNDEFINED)

# Subdirectory scope raise.
set(SUBDIR_UNDEFINED 1)
add_subdirectory(SubDirScope)
if(DEFINED SUBDIR_UNDEFINED)
  FAILED("Subdir Undefine Test" "(${SUBDIR_UNDEFINED})")
else(DEFINED SUBDIR_UNDEFINED)
  PASS("Subdir Undefine Test" "(${SUBDIR_UNDEFINED})")
endif(DEFINED SUBDIR_UNDEFINED)
if(DEFINED SUBDIR_DEFINED)
  PASS("Subdir Define Test" "(${SUBDIR_DEFINED})")
else(DEFINED SUBDIR_DEFINED)
  FAILED("Subdir Define Test" "(${SUBDIR_DEFINED})")
endif(DEFINED SUBDIR_DEFINED)

# Test function-scoped directory.
function(ADD_SUBDIR2 dir)
  add_subdirectory("${dir}" "${dir}2")
  # The parent scope sets in the subdir should be visible here.
  if(DEFINED SUBDIR_UNDEFINED)
    FAILED("Subdir Function Undefine Test 1" "(${SUBDIR_UNDEFINED})")
  else(DEFINED SUBDIR_UNDEFINED)
    PASS("Subdir Function Undefine Test 1" "(${SUBDIR_UNDEFINED})")
  endif(DEFINED SUBDIR_UNDEFINED)
  if(DEFINED SUBDIR_DEFINED)
    PASS("Subdir Function Define Test 1" "(${SUBDIR_DEFINED})")
  else(DEFINED SUBDIR_DEFINED)
    FAILED("Subdir Function Define Test 1" "(${SUBDIR_DEFINED})")
  endif(DEFINED SUBDIR_DEFINED)
endfunction(ADD_SUBDIR2)

# Reset test variables.
set(SUBDIR_UNDEFINED 1)
set(SUBDIR_DEFINED)

# Run test function.
ADD_SUBDIR2(SubDirScope)

# The parent scope sets in the subdir should not be visible here.
if(DEFINED SUBDIR_UNDEFINED)
  PASS("Subdir Function Undefine Test 2" "(${SUBDIR_UNDEFINED})")
else(DEFINED SUBDIR_UNDEFINED)
  FAILED("Subdir Function Undefine Test 2" "(${SUBDIR_UNDEFINED})")
endif(DEFINED SUBDIR_UNDEFINED)
if(DEFINED SUBDIR_DEFINED)
  FAILED("Subdir Function Define Test 2" "(${SUBDIR_DEFINED})")
else(DEFINED SUBDIR_DEFINED)
  PASS("Subdir Function Define Test 2" "(${SUBDIR_DEFINED})")
endif(DEFINED SUBDIR_DEFINED)

add_executable(FunctionTest functionTest.c)

# Use the PROJECT_LABEL property: in IDEs, the project label should appear
# in the UI rather than the target name. If this were a good test of the
# property rather than just a smoke test, it would verify that the label
# actually appears in the UI of the IDE... Or at least that the text appears
# somewhere in the generated project files.
set_property(TARGET miniFunctionTest
  PROPERTY PROJECT_LABEL "Test de Fonctionnement")
