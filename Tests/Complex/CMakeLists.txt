#
# A more complex test case
#
PROJECT (Complex)

#
# Use the ansi CXX compile flag for building cmake
#
IF (CMAKE_ANSI_CXXFLAGS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  
ENDIF (CMAKE_ANSI_CXXFLAGS)

ADD_DEFINITIONS(-DCMAKE_IS_FUN)

INCLUDE(${Complex_SOURCE_DIR}/VarTests.txt)

LOAD_CACHE(${Complex_SOURCE_DIR})

CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/cmTestConfigure.h.in
  ${Complex_BINARY_DIR}/cmTestConfigure.h)

INCLUDE_DIRECTORIES(
  ${Complex_BINARY_DIR}
  ${Complex_SOURCE_DIR}/Library
  ${Complex_SOURCE_DIR}/../../Source
)

LINK_DIRECTORIES(
  ${Complex_BINARY_DIR}/Library
)

INCLUDE_REGULAR_EXPRESSION("^(cmTest|file|sharedFile).*$")

SET (LIBRARY_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all executables.")

#
# Where will executable tests be written ?
#
IF (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH .)
ENDIF (EXECUTABLE_OUTPUT_PATH)

SUBDIRS(Library Executable)
SUBDIR_DEPENDS(Executable Library)

#
# Exec program
# Increase coverage.
#
OPTION(EXEC_PROGRAM "Test EXEC_PROGRAM" ON)
IF (EXEC_PROGRAM) 
  EXEC_PROGRAM("echo EXEC_PROGRAM")
ENDIF (EXEC_PROGRAM) 

#
# More coverage
#
ABSTRACT_FILES(
  ExtraSources/file1.cxx
)

INSTALL_FILES(/tmp .h ${Complex_BINARY_DIR}/cmTestConfigure.h)

#
# Create directory.
# The 'complex' executable will then test if this dir exists,
# sadly it won't be able to remove it.
#
MAKE_DIRECTORY("${Complex_BINARY_DIR}/make_dir")

#
# Test Cable
#
CABLE_CLASS_SET(Float float)
CABLE_CLASS_SET(Mesh "itk::Mesh<$Float>")

