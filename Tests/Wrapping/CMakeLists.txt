#
# Wrapping
#
PROJECT (Wrapping)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
     ${Wrapping_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${Wrapping_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all executables.")

#
# Where will executable tests be written ?
#
IF (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH .)
ENDIF (EXECUTABLE_OUTPUT_PATH)

#
# Link to CMake lib
#
FIND_LIBRARY(CMAKE_LIB
             CMakeLib 
             ${Wrapping_BINARY_DIR}/../../Source
             ${Wrapping_BINARY_DIR}/../../Source/Debug
             ${Wrapping_BINARY_DIR}/../../Source/Release
             ${Wrapping_BINARY_DIR}/../../Source/MinSizeRel
             ${Wrapping_BINARY_DIR}/../../Source/RelWithDebInfo)

LINK_LIBRARIES(${CMAKE_LIB})

#
# Test Cable
#
CABLE_CLASS_SET(Float float)
CABLE_CLASS_SET(Mesh "itk::Mesh<$Float>")

#
# Add exe
#
ADD_EXECUTABLE(wrapping wrapping.cxx)

