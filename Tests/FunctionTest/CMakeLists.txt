# a simple C only test case
PROJECT (FunctionTest)

SET(CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")

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

# test recursion and return via raise_scope
function (factorial argument result)
  if (argument LESS 2)
    set (${result} 1)
  else (argument LESS 2)
    math (EXPR temp "${argument} - 1")
    factorial (${temp} tresult)
    math (EXPR ${result} "${argument}*${tresult}")
  endif (argument LESS 2)
  raise_scope (${result})
endfunction (factorial)

factorial (5 fresult)
if (fresult EQUAL 120)
  pass("factorial")
else (fresult EQUAL 120)
  failed ("factorial, computed ${fresult} instead of 120")
endif (fresult EQUAL 120)



# case test
FUNCTION(strange_function m)
  SET(${m} strange_function)
  RAISE_SCOPE(${m})
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

ADD_EXECUTABLE(FunctionTest functionTest.c)
