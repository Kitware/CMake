PROJECT(LoadedCommand)

# set a definition
SET (TEST_COMMAND_TEST1 1)

# try to compile the command
TRY_COMPILE(COMPILE_OK 
    ${LoadedCommand_BINARY_DIR}/CMakeCommands 
    ${LoadedCommand_SOURCE_DIR}/CMakeCommands 
    CMAKE_LOADED_COMMANDS cmCMAKE_TEST_COMMAND)

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

