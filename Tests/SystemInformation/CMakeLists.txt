PROJECT(DumpInformation)
INCLUDE_DIRECTORIES(${DumpInformation_BINARY_DIR})
CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/SystemInformation.in 
${DumpInformation_BINARY_DIR}/SystemInformation.out)
CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/DumpInformation.h.in 
${DumpInformation_BINARY_DIR}/DumpInformation.h)
ADD_EXECUTABLE(DumpInformation DumpInformation.cxx)

WRITE_FILE(${DumpInformation_BINARY_DIR}/AllVariables.txt "")
GET_CMAKE_PROPERTY(VARS VARIABLES)
FOREACH(var ${VARS})
  WRITE_FILE(${DumpInformation_BINARY_DIR}/AllVariables.txt 
             "${var} \"${${var}}\"" APPEND)
  #MESSAGE(STATUS "Variable ${var} \"${${var}}\"")
ENDFOREACH(var ${VARS})

