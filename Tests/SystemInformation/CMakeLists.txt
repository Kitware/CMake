PROJECT(DumpInformation)
INCLUDE_DIRECTORIES(${DumpInformation_BINARY_DIR})
CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/SystemInformation.in 
${DumpInformation_BINARY_DIR}/SystemInformation.out)
CONFIGURE_FILE(${DumpInformation_SOURCE_DIR}/DumpInformation.h.in 
${DumpInformation_BINARY_DIR}/DumpInformation.h)
ADD_EXECUTABLE(DumpInformation DumpInformation.cxx)

GET_CMAKE_PROPERTY(VARS VARIABLES)
FOREACH(var ${VARS})
  MESSAGE(STATUS "Variable ${var} \"${${var}}\"")
ENDFOREACH(var ${VARS})

