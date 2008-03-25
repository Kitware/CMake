cmake_minimum_required (VERSION 2.6)
PROJECT(LoadCommand)

# set a definition
SET (TEST_COMMAND_TEST1 1)

INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
CHECK_TYPE_SIZE(char  SIZEOF_CHAR)
CHECK_TYPE_SIZE(short  SIZEOF_SHORT)

INCLUDE (CheckFunctionExists)
CHECK_FUNCTION_EXISTS(printf        HAVE_PRINTF)
CHECK_FUNCTION_EXISTS(vsblabla      HAVE_VSBLABLA)

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE("sys/prctl.h"    HAVE_SYS_PRCTL_H)

INCLUDE (${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
CHECK_LIBRARY_EXISTS(m ceil "" HAVE_LIBM)

CONFIGURE_FILE(${LoadCommand_SOURCE_DIR}/LoadedCommand.h.in
               ${LoadCommand_BINARY_DIR}/LoadedCommand.h)

INCLUDE_DIRECTORIES(${LoadCommand_BINARY_DIR})

# try to compile the command
# make sure it is not already loaded
IF(COMMAND CMAKE_TEST_COMMAND)
ELSE(COMMAND CMAKE_TEST_COMMAND)
  TRY_COMPILE(COMPILE_OK 
    ${LoadCommand_BINARY_DIR}/CMakeCommands 
    ${LoadCommand_SOURCE_DIR}/CMakeCommands 
    CMAKE_LOADED_COMMANDS CMAKE_FLAGS -DMUDSLIDE_TYPE:STRING=MUCHO
    OUTPUT_VARIABLE OUTPUT )
# do another TRY_COMPILE to get around make
# problem on hp
  TRY_COMPILE(COMPILE_OK 
    ${LoadCommand_BINARY_DIR}/CMakeCommands 
    ${LoadCommand_SOURCE_DIR}/CMakeCommands 
    CMAKE_LOADED_COMMANDS CMAKE_FLAGS -DMUDSLIDE_TYPE:STRING=MUCHO
    OUTPUT_VARIABLE OUTPUT )
ENDIF(COMMAND CMAKE_TEST_COMMAND)

MESSAGE("Output from try compile: ${OUTPUT}")

# if the compile was OK, try loading the command
IF (COMPILE_OK)
  LOAD_COMMAND(CMAKE_TEST_COMMAND 
    ${LoadCommand_BINARY_DIR}/CMakeCommands 
    ${LoadCommand_BINARY_DIR}/CMakeCommands/Debug
    ${LoadCommand_BINARY_DIR}/CMakeCommands/Development
    )
  # if the command loaded, execute the command
  IF (COMMAND CMAKE_TEST_COMMAND)
    CMAKE_TEST_COMMAND(
      "${LoadCommand_SOURCE_DIR}/LoadedCommand.cxx.in"
      "${LoadCommand_BINARY_DIR}/LoadedCommand2.cxx.in"
      "${LoadCommand_BINARY_DIR}/LoadedCommand3.cxx"
      )
  ENDIF (COMMAND CMAKE_TEST_COMMAND)
ELSE (COMPILE_OK)
  MESSAGE("failed to compile CMAKE_LOADED_COMMANDS")
ENDIF (COMPILE_OK)

# TEST_DEF is set by the loaded command cmTestCommand.c
IF (TEST_DEF AND SOME_CACHE_VARIABLE AND TEST_EXEC)
  ADD_DEFINITIONS(-DCMAKE_IS_FUN)
ENDIF (TEST_DEF AND SOME_CACHE_VARIABLE AND TEST_EXEC)

