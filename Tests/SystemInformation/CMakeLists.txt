PROJECT(DumpInformation)

INCLUDE_DIRECTORIES("This does not exists")
GET_DIRECTORY_PROPERTY(incl INCLUDE_DIRECTORIES)
SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${DumpInformation_BINARY_DIR};${DumpInformation_SOURCE_DIR}")

MESSAGE("To prevent CTest from stripping output, you have to display: CTEST_FULL_OUTPUT")


CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/SystemInformation.in 
${DumpInformation_BINARY_DIR}/SystemInformation.out)
CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/DumpInformation.h.in 
${DumpInformation_BINARY_DIR}/DumpInformation.h)
ADD_EXECUTABLE(DumpInformation DumpInformation.cxx)

MACRO(FOO args)
  MESSAGE("Test macro")
ENDMACRO(FOO)

FOO(lala)

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt "")
GET_CMAKE_PROPERTY(res VARIABLES)
FOREACH(var ${res})
  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt 
             "${var} \"${${var}}\"\n")
ENDFOREACH(var ${res})

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/AllCommands.txt "")
GET_CMAKE_PROPERTY(res COMMANDS)
FOREACH(var ${res})
  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllCommands.txt 
             "${var}\n")
ENDFOREACH(var ${res})

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/AllMacros.txt "")
GET_CMAKE_PROPERTY(res MACROS)
FOREACH(var ${res})
  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllMacros.txt 
             "${var}\n")
ENDFOREACH(var ${res})

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/OtherProperties.txt "")
GET_DIRECTORY_PROPERTY(res INCLUDE_DIRECTORIES)
FOREACH(var ${res})
  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/OtherProperties.txt
    "INCLUDE_DIRECTORY: ${var}\n")
ENDFOREACH(var)

GET_DIRECTORY_PROPERTY(res LINK_DIRECTORIES)
FOREACH(var ${res})
  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/OtherProperties.txt
    "LINK_DIRECTORIES: ${var}\n")
ENDFOREACH(var)

GET_DIRECTORY_PROPERTY(res INCLUDE_REGULAR_EXPRESSION)
FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/OtherProperties.txt
  "INCLUDE_REGULAR_EXPRESSION: ${res}\n")


