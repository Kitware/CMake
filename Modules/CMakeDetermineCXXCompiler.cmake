# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.
# use environment variable CXX first if defined by user, next use 
# the cmake variable CMAKE_GENERATOR_CXX which can be defined by a generator
# as a default compiler

IF(NOT CMAKE_CXX_COMPILER)
  SET(CMAKE_CXX_COMPILER_INIT NOTFOUND)

  # if the user has specified CC via the environment, then use that without checking
  IF($ENV{CXX} MATCHES ".+")
    GET_FILENAME_COMPONENT(CMAKE_CXX_COMPILER_INIT $ENV{CXX} PROGRAM PROGRAM_ARGS CMAKE_CXX_FLAGS_ENV_INIT)
    IF(EXISTS ${CMAKE_CXX_COMPILER_INIT})
    ELSE(EXISTS ${CMAKE_CXX_COMPILER_INIT})
      MESSAGE(SEND_ERROR "Could not find compiler set in environment variable CXX:\n$ENV{CXX}.\n${CMAKE_CXX_COMPILER_INIT}")
    ENDIF(EXISTS ${CMAKE_CXX_COMPILER_INIT})
  ENDIF($ENV{CXX} MATCHES ".+")

  IF(CMAKE_GENERATOR_CXX)
    IF(NOT CMAKE_CXX_COMPILER_INIT)
      SET(CMAKE_CXX_COMPILER_INIT ${CMAKE_GENERATOR_CXX})
    ENDIF(NOT CMAKE_CXX_COMPILER_INIT)
  ENDIF(CMAKE_GENERATOR_CXX)

  IF(NOT CMAKE_CXX_COMPILER_INIT)
  # if not in the envionment then search for the compiler in the path
    SET(CMAKE_CXX_COMPILER_LIST c++ g++ CC aCC cl bcc )
    FIND_PROGRAM(CMAKE_CXX_COMPILER_FULLPATH NAMES ${CMAKE_CXX_COMPILER_LIST})
    GET_FILENAME_COMPONENT(CMAKE_CXX_COMPILER_INIT
                           ${CMAKE_CXX_COMPILER_FULLPATH} NAME)
  ENDIF(NOT CMAKE_CXX_COMPILER_INIT)
  SET(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER_INIT} 
      CACHE STRING "C++ compiler") 
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
