
# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.
# use environment variable CCC first if defined by user, next use 
# the cmake variable CMAKE_GENERATOR_CC which can be defined by a generator
# as a default compiler
IF(NOT CMAKE_RC_COMPILER)
  # prefer the environment variable CC
  IF($ENV{RC} MATCHES ".+")
    GET_FILENAME_COMPONENT(CMAKE_RC_COMPILER_INIT $ENV{RC} PROGRAM PROGRAM_ARGS CMAKE_RC_FLAGS_ENV_INIT)
    IF(CMAKE_RC_FLAGS_ENV_INIT)
      SET(CMAKE_RC_COMPILER_ARG1 "${CMAKE_RC_FLAGS_ENV_INIT}" CACHE STRING "First argument to RC compiler")
    ENDIF(CMAKE_RC_FLAGS_ENV_INIT)
    IF(EXISTS ${CMAKE_RC_COMPILER_INIT})
    ELSE(EXISTS ${CMAKE_RC_COMPILER_INIT})
      MESSAGE(FATAL_ERROR "Could not find compiler set in environment variable RC:\n$ENV{RC}.") 
    ENDIF(EXISTS ${CMAKE_RC_COMPILER_INIT})
  ENDIF($ENV{RC} MATCHES ".+")
  
  # next try prefer the compiler specified by the generator
  IF(CMAKE_GENERATOR_RC) 
    IF(NOT CMAKE_RC_COMPILER_INIT)
      SET(CMAKE_RC_COMPILER_INIT ${CMAKE_GENERATOR_RC})
    ENDIF(NOT CMAKE_RC_COMPILER_INIT)
  ENDIF(CMAKE_GENERATOR_RC)
  
  
  # if no compiler has been specified yet, then look for one
  IF(NOT CMAKE_RC_COMPILER_INIT)
    SET(CMAKE_RC_COMPILER_LIST rc)
    FIND_PROGRAM(CMAKE_RC_COMPILER_FULLPATH NAMES ${CMAKE_RC_COMPILER_LIST} )
    GET_FILENAME_COMPONENT(CMAKE_RC_COMPILER_INIT
      ${CMAKE_RC_COMPILER_FULLPATH} NAME)
    SET(CMAKE_RC_COMPILER_FULLPATH "${CMAKE_RC_COMPILER_FULLPATH}" 
      CACHE INTERNAL "full path to the compiler cmake found")
  ENDIF(NOT CMAKE_RC_COMPILER_INIT)

  SET(CMAKE_RC_COMPILER ${CMAKE_RC_COMPILER_INIT} CACHE STRING "RC compiler")
ENDIF(NOT CMAKE_RC_COMPILER)

MARK_AS_ADVANCED(CMAKE_RC_COMPILER)  


# configure variables set in this file for fast reload later on
CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeRCCompiler.cmake.in 
               ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeRCCompiler.cmake IMMEDIATE)
SET(CMAKE_RC_COMPILER_ENV_VAR "RC")
