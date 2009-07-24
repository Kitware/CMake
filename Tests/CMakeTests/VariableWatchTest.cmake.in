MESSAGE("Start")

VARIABLE_WATCH(TESTVAR MESSAGE)
VARIABLE_WATCH(TESTVAR1)

macro(testwatch var access file stack)
  MESSAGE("There was a ${access} access done on the variable: ${var} in file ${file}")
  MESSAGE("List file stack is: ${stack}")
  set(${var}_watched 1)
endmacro(testwatch)

VARIABLE_WATCH(somevar testwatch)

set(TESTVAR1 "1")
set(TESTVAR "1")
set(TESTVAR1 "0")
set(TESTVAR "0")


message("Variable: ${somevar}")
if(NOT somevar_watched)
  message(SEND_ERROR "'somevar' watch failed!")
endif()
set(somevar_watched)

set(somevar "1")
message("Variable: ${somevar}")
if(NOT somevar_watched)
  message(SEND_ERROR "'somevar' watch failed!")
endif()
remove(somevar)
