
# determine the compiler to use for C++ programs
# NOTE, a generator may set CMAKE_CXX_COMPILER before
# loading this file to force a compiler.
# use environment variable CXX first if defined by user, next use 
# the cmake variable CMAKE_GENERATOR_CXX which can be defined by a generator
# as a default compiler
#
# Sets the following variables:
#   CMAKE_CXX_COMPILER
#   CMAKE_COMPILER_IS_GNUCXX
#   CMAKE_AR
#   CMAKE_RANLIB


IF(NOT CMAKE_CXX_COMPILER)
  SET(CMAKE_CXX_COMPILER_INIT NOTFOUND)

  # prefer the environment variable CXX
  IF($ENV{CXX} MATCHES ".+")
    GET_FILENAME_COMPONENT(CMAKE_CXX_COMPILER_INIT $ENV{CXX} PROGRAM PROGRAM_ARGS CMAKE_CXX_FLAGS_ENV_INIT)
    IF(CMAKE_CXX_FLAGS_ENV_INIT)
      SET(CMAKE_CXX_COMPILER_ARG1 "${CMAKE_CXX_FLAGS_ENV_INIT}" CACHE STRING "First argument to CXX compiler")
    ENDIF(CMAKE_CXX_FLAGS_ENV_INIT)
    IF(EXISTS ${CMAKE_CXX_COMPILER_INIT})
    ELSE(EXISTS ${CMAKE_CXX_COMPILER_INIT})
      MESSAGE(FATAL_ERROR "Could not find compiler set in environment variable CXX:\n$ENV{CXX}.\n${CMAKE_CXX_COMPILER_INIT}")
    ENDIF(EXISTS ${CMAKE_CXX_COMPILER_INIT})
  ENDIF($ENV{CXX} MATCHES ".+")

  # next prefer the generator specified compiler
  IF(CMAKE_GENERATOR_CXX)
    IF(NOT CMAKE_CXX_COMPILER_INIT)
      SET(CMAKE_CXX_COMPILER_INIT ${CMAKE_GENERATOR_CXX})
    ENDIF(NOT CMAKE_CXX_COMPILER_INIT)
  ENDIF(CMAKE_GENERATOR_CXX)

  # finally list compilers to try
  IF(CMAKE_CXX_COMPILER_INIT)
    SET(CMAKE_CXX_COMPILER_LIST ${CMAKE_CXX_COMPILER_INIT})
  ELSE(CMAKE_CXX_COMPILER_INIT)
    SET(CMAKE_CXX_COMPILER_LIST c++ g++ CC aCC cl bcc xlC)
  ENDIF(CMAKE_CXX_COMPILER_INIT)

  # Find the compiler.
  FIND_PROGRAM(CMAKE_CXX_COMPILER NAMES ${CMAKE_CXX_COMPILER_LIST} DOC "C++ compiler")
  IF(CMAKE_CXX_COMPILER_INIT AND NOT CMAKE_CXX_COMPILER)
    SET(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_INIT}" CACHE FILEPATH "C++ compiler" FORCE)
  ENDIF(CMAKE_CXX_COMPILER_INIT AND NOT CMAKE_CXX_COMPILER)
ENDIF(NOT CMAKE_CXX_COMPILER)
MARK_AS_ADVANCED(CMAKE_CXX_COMPILER)

GET_FILENAME_COMPONENT(COMPILER_LOCATION "${CMAKE_CXX_COMPILER}" PATH)

FIND_PROGRAM(CMAKE_AR NAMES ar PATHS ${COMPILER_LOCATION})
MARK_AS_ADVANCED(CMAKE_AR)

FIND_PROGRAM(CMAKE_RANLIB NAMES ranlib)
IF(NOT CMAKE_RANLIB)
   SET(CMAKE_RANLIB : CACHE INTERNAL "noop for ranlib")
ENDIF(NOT CMAKE_RANLIB)
MARK_AS_ADVANCED(CMAKE_RANLIB)

# This block was used before the compiler was identified by building a
# source file.  Unless g++ crashes when building a small C++
# executable this should no longer be needed.
#
# The g++ that comes with BeOS 5 segfaults if you run "g++ -E"
#  ("gcc -E" is fine), which throws up a system dialog box that hangs cmake
#  until the user clicks "OK"...so for now, we just assume it's g++.
# IF(BEOS)
#   SET(CMAKE_COMPILER_IS_GNUCXX 1)
#   SET(CMAKE_COMPILER_IS_GNUCXX_RUN 1)
# ENDIF(BEOS)

# Build a small source file to identify the compiler.
IF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  SET(CMAKE_CXX_COMPILER_ID_RUN 1)
  SET(CMAKE_CXX_PLATFORM_ID "Windows")

  # TODO: Set the compiler id.  It is probably MSVC but
  # the user may be using an integrated Intel compiler.
  # SET(CMAKE_CXX_COMPILER_ID "MSVC")
ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
IF(NOT CMAKE_CXX_COMPILER_ID_RUN)
  SET(CMAKE_CXX_COMPILER_ID_RUN 1)

  # Try to identify the compiler.
  SET(CMAKE_CXX_COMPILER_ID)
  INCLUDE(${CMAKE_ROOT}/Modules/CMakeDetermineCompilerId.cmake)
  CMAKE_DETERMINE_COMPILER_ID(CXX CXXFLAGS ${CMAKE_ROOT}/Modules/CMakeCXXCompilerId.cpp)

  # Set old compiler and platform id variables.
  IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    SET(CMAKE_COMPILER_IS_GNUCXX 1)
  ENDIF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  IF("${CMAKE_CXX_PLATFORM_ID}" MATCHES "MinGW")
    SET(CMAKE_COMPILER_IS_MINGW 1)
  ELSEIF("${CMAKE_CXX_PLATFORM_ID}" MATCHES "Cygwin")
    SET(CMAKE_COMPILER_IS_CYGWIN 1)
  ENDIF("${CMAKE_CXX_PLATFORM_ID}" MATCHES "MinGW")
ENDIF(NOT CMAKE_CXX_COMPILER_ID_RUN)

# configure all variables set in this file
CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeCXXCompiler.cmake.in 
               ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeCXXCompiler.cmake IMMEDIATE)

SET(CMAKE_CXX_COMPILER_ENV_VAR "CXX")
