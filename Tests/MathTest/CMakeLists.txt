PROJECT(MathTest)


# Expression test

SET(expressions
  "5 * ( 3 + 4)"
  "(1 | 2 | 4 | 8) & 16"
  "1 +(3*4) + 10 >> 2"
  "10000 / 20 / 4"
  "10000 / (20 / 4)"
  )

SET(FILE_EXPRESSIONS "")
FOREACH(expression
    ${expressions})
  MATH(EXPR expr "${expression}")
  SET(FILE_EXPRESSIONS "${FILE_EXPRESSIONS}TEST_EXPRESSION(${expression}, ${expr})\n")
ENDFOREACH(expression)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/MathTestTests.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/MathTestTests.h"
  @ONLY)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")
ADD_EXECUTABLE(MathTestExec MathTestExec.cxx)

