# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.

FIND_PROGRAM(CMAKE_C_COMPILER NAMES $ENV{CC} gcc cc cl bcc PATHS /bin /usr/bin /usr/local/bin )

# test to see if the c compiler is gnu
EXEC_PROGRAM(${CMAKE_C_COMPILER} ARGS -E ${CMAKE_ROOT}/Modules/CMakeTestGNU.c OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT RETURN_VALUE CMAKE_COMPILER_RETURN)

IF(NOT CMAKE_COMPILER_RETURN)
   IF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
      SET(CMAKE_COMPILER_IS_GNUGCC 1)
   ENDIF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
ENDIF(NOT CMAKE_COMPILER_RETURN)

