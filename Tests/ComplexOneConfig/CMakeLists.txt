# a simple test case
PROJECT (Complex)

# use the ansi CXX compile flag for building cmake
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  

ADD_DEFINITIONS(-DCMAKE_IS_FUN)
SUBDIRS(Library Executable)
SUBDIR_DEPENDS(Executable Library)

INCLUDE(${Complex_SOURCE_DIR}/VarTests.txt)

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

SET (LIBRARY_OUTPUT_PATH ${Complex_BINARY_DIR}/bin/ CACHE PATH "Single output directory for building all libraries.")
SET (EXECUTABLE_OUTPUT_PATH ${Complex_BINARY_DIR}/bin/ CACHE PATH "Single output directory for building all executables.")
