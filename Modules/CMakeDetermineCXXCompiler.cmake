# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.
# use environment variable CXX first if defined by user, next use 
# the cmake variable CMAKE_GENERATOR_CXX which can be defined by a generator
# as a default compiler

IF(NOT CMAKE_CXX_COMPILER)
  FIND_PROGRAM(CMAKE_CXX_COMPILER_FULLPATH NAMES $ENV{CXX} ${CMAKE_GENERATOR_CXX} c++ g++ CC aCC cl bcc )
  GET_FILENAME_COMPONENT(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER_FULLPATH} NAME)
  SET(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER} CACHE STRING "C++ compiler")
ENDIF(NOT CMAKE_CXX_COMPILER)
MARK_AS_ADVANCED(CMAKE_CXX_COMPILER)

# set this to notfound right after so that it is searched for each time this
# file is included
SET(CMAKE_CXX_COMPILER_FULLPATH NOTFOUND CACHE INTERNAL "full path to cxx compiler" FORCE)


# test to see if the cxx compiler is gnu
EXEC_PROGRAM(${CMAKE_CXX_COMPILER} ARGS -E ${CMAKE_ROOT}/Modules/CMakeTestGNU.c OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT RETURN_VALUE CMAKE_COMPILER_RETURN)
IF(NOT CMAKE_COMPILER_RETURN)
   IF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
      SET(CMAKE_COMPILER_IS_GNUCXX 1)
   ENDIF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
ENDIF(NOT CMAKE_COMPILER_RETURN)

# configure all variables set in this file
CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeCXXCompiler.cmake.in 
               ${PROJECT_BINARY_DIR}/CMakeCXXCompiler.cmake IMMEDIATE)
MARK_AS_ADVANCED(CMAKE_CXX_COMPILER_FULLPATH)
