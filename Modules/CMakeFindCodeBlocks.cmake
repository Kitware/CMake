# This file is included in CMakeSystemSpecificInformation.cmake if
# the CodeBlocks extra generator has been selected.

FIND_PROGRAM(CMAKE_CODEBLOCKS_EXECUTABLE NAMES codeblocks DOC "The CodeBlocks executable")

IF(CMAKE_CODEBLOCKS_EXECUTABLE)
   SET(CMAKE_OPEN_PROJECT_COMMAND "${CMAKE_CODEBLOCKS_EXECUTABLE} <PROJECT_FILE>" )
ENDIF(CMAKE_CODEBLOCKS_EXECUTABLE)

