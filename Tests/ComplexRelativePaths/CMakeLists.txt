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

# use the ANSI flags to compile complex as it uses cmSystemTools from
# the cmake library which requires ansi
IF (CMAKE_ANSI_CXXFLAGS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  
ENDIF (CMAKE_ANSI_CXXFLAGS)

IF (CMAKE_ANSI_CFLAGS)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ANSI_CFLAGS}")  
ENDIF (CMAKE_ANSI_CFLAGS)

#
# Include vars from a file and from a cache
#
IF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)
  INCLUDE(${Complex_SOURCE_DIR}/VarTests.cmake)
ENDIF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)

LOAD_CACHE(${Complex_SOURCE_DIR}
           EXCLUDE
             CACHE_TEST_VAR_EXCLUDED
           INCLUDE_INTERNALS
             CACHE_TEST_VAR_INTERNAL)

#
# Configure file 
# (plug vars to #define so that they can be tested)
#
CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/cmTestConfigure.h.in
  ${Complex_BINARY_DIR}/cmTestConfigure.h)

#
# Specify include and lib dirs 
# (BEFORE is for coverage)
#
INCLUDE_DIRECTORIES(
  ${Complex_SOURCE_DIR}/Library
  ${Complex_SOURCE_DIR}/../../Source
)

INCLUDE_DIRECTORIES(BEFORE
  ${Complex_BINARY_DIR}
)

INCLUDE_REGULAR_EXPRESSION("^(cmTest|file|sharedFile).*$" ".*")

LINK_DIRECTORIES(
  ${Complex_BINARY_DIR}/Library
)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all executables.")

#
# Create the libs and the main exe
#
SUBDIRS(Library Executable)
SUBDIR_DEPENDS(Executable Library)

#
# Exec program (TODO: test a result)
# Increase coverage.
#
MESSAGE("Trying to increase coverage...")
OPTION(NO_EXEC_PROGRAM "Do not test EXEC_PROGRAM" 0)
IF (NOT NO_EXEC_PROGRAM)
  EXEC_PROGRAM("echo NO_EXEC_PROGRAM" "${Complex_BINARY_DIR}")
ELSE (NOT NO_EXEC_PROGRAM) 
  MESSAGE("Set this option ON")
ENDIF (NOT NO_EXEC_PROGRAM) 

MARK_AS_ADVANCED(NO_EXEC_PROGRAM)
MARK_AS_ADVANCED(CLEAR NO_EXEC_PROGRAM)

#
# Create directory.
# The 'complex' executable will then test if this dir exists,
# sadly it won't be able to remove it.
#
MAKE_DIRECTORY("${Complex_BINARY_DIR}/make_dir")
