PROJECT(DumpInformation)
INCLUDE_DIRECTORIES(${DumpInformation_BINARY_DIR})
CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/SystemInformation.in 
${DumpInformation_BINARY_DIR}/SystemInformation.out)
CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/DumpInformation.h.in 
${DumpInformation_BINARY_DIR}/DumpInformation.h)
ADD_EXECUTABLE(DumpInformation DumpInformation.cxx)

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

