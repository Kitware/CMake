# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.

FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES $ENV{CXX} c++ g++ CC aCC cl bcc PATHS /bin /usr/bin /usr/local/bin )

# test to see if the cxx compiler is gnu
EXEC_PROGRAM(${CMAKE_CXX_COMPILER} ARGS -E ${CMAKE_ROOT}/Modules/CMakeTestGNU.c OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT RETURN_VALUE CMAKE_COMPILER_RETURN)

IF(NOT CMAKE_COMPILER_RETURN)
   IF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
      SET(CMAKE_COMPILER_IS_GNUGXX 1)
   ENDIF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
ENDIF(NOT CMAKE_COMPILER_RETURN)
