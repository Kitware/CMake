PROJECT(LoadedCommand)

# set a definition
SET (TEST_COMMAND_TEST1 1)

INCLUDE (${CMAKE_ROOT}/Modules/CheckSizeOf.cmake)
CHECK_TYPE_SIZE(char  SIZEOF_CHAR)
CHECK_TYPE_SIZE(short SIZEOF_SHORT)
CHECK_TYPE_SIZE(int   SIZEOF_INT)
CHECK_TYPE_SIZE(long  SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE(__int64   SIZEOF___INT64)

INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
CHECK_FUNCTION_EXISTS(vsprintf      HAVE_VSPRINTF)
CHECK_FUNCTION_EXISTS(vsnprintf     HAVE_VSNPRINTF)
CHECK_FUNCTION_EXISTS(vsblabla      HAVE_VSBLABLA)
CHECK_FUNCTION_EXISTS(gethostbyname HAVE_GETHOSTBYNAME)
CHECK_FUNCTION_EXISTS(connect       HAVE_CONNECT)
CHECK_FUNCTION_EXISTS(remove        HAVE_REMOVE)
CHECK_FUNCTION_EXISTS(shmat         HAVE_SHMAT)

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE("sys/prctl.h"    HAVE_SYS_PRCTL_H)
CHECK_INCLUDE_FILE("sys/prafasdfctl.h"    HAVE_SYS_PRAFASDFCTL_H)

INCLUDE (${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
CHECK_LIBRARY_EXISTS(m ceil "" HAVE_LIBM)

CONFIGURE_FILE(${LoadedCommand_SOURCE_DIR}/LoadedCommand.h.in
               ${LoadedCommand_BINARY_DIR}/LoadedCommand.h)

INCLUDE_DIRECTORIES(${LoadedCommand_BINARY_DIR})

# try to compile the command
TRY_COMPILE(COMPILE_OK 
    ${LoadedCommand_BINARY_DIR}/CMakeCommands 
    ${LoadedCommand_SOURCE_DIR}/CMakeCommands 
    CMAKE_LOADED_COMMANDS CMAKE_FLAGS -DMUDSLIDE_TYPE:STRING=MUCHO)

# if the compile was OK, try loading the command
IF (COMPILE_OK)
  LOAD_COMMAND(CMAKE_TEST_COMMAND 
    ${LoadedCommand_BINARY_DIR}/CMakeCommands 
    ${LoadedCommand_BINARY_DIR}/CMakeCommands/Debug
    )
  # if the command loaded, execute the command
  IF (COMMAND CMAKE_TEST_COMMAND)
    CMAKE_TEST_COMMAND()
  ENDIF (COMMAND CMAKE_TEST_COMMAND)
ELSE (COMPILE_OK)
  MESSAGE("failed to compile CMAKE_LOADED_COMMANDS")
ENDIF (COMPILE_OK)

IF (TEST_DEF)
  ADD_DEFINITIONS(-DCMAKE_IS_FUN)
ENDIF (TEST_DEF)

ADD_EXECUTABLE (LoadedCommand LoadedCommand.cxx)
