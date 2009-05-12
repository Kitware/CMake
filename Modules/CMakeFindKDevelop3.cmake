# This file is included in CMakeSystemSpecificInformation.cmake if
# the KDevelop3 extra generator has been selected.

FIND_PROGRAM(CMAKE_KDEVELOP3_EXECUTABLE NAMES kdevelop DOC "The KDevelop3 executable")

IF(CMAKE_KDEVELOP3_EXECUTABLE)
   SET(CMAKE_OPEN_PROJECT_COMMAND "${CMAKE_KDEVELOP3_EXECUTABLE} <PROJECT_FILE>" )
ENDIF(CMAKE_KDEVELOP3_EXECUTABLE)

