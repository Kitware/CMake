CMAKE_MINIMUM_REQUIRED(VERSION 1.3)
#
# Create exe. 
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEST_CXX_FLAGS")

# Link to CMake lib
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source)
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source/kwsys)
# Use LINK_LIBRARIES instead of TARGET_LINK_LIBRARIES to 
SET(COMPLEX_LIBS CMakeTestLibrary;CMakeTestLibraryShared;CMakeTestCLibraryShared)
LINK_LIBRARIES(${COMPLEX_LIBS})


ADD_EXECUTABLE(complex complex)
IF (UNIX)
  TARGET_LINK_LIBRARIES(complex CMakeLib cmsys ${CMAKE_DL_LIBS})
ELSE(UNIX)
  TARGET_LINK_LIBRARIES(complex CMakeLib cmsys)
  IF (NOT BORLAND)
    TARGET_LINK_LIBRARIES(complex rpcrt4.lib)
  ENDIF(NOT BORLAND)
ENDIF (UNIX)

SET_SOURCE_FILES_PROPERTIES(complex
  COMPILE_FLAGS "-DFILE_HAS_EXTRA_COMPILE_FLAGS"
  OBJECT_DEPENDS ${Complex_BINARY_DIR}/cmTestGeneratedHeader.h
)

ADD_CUSTOM_COMMAND(
  TARGET complex
  SOURCE ${Complex_SOURCE_DIR}/cmTestGeneratedHeader.h.in
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy ${Complex_SOURCE_DIR}/cmTestGeneratedHeader.h.in
               ${Complex_BINARY_DIR}/cmTestGeneratedHeader.h
  OUTPUTS ${Complex_BINARY_DIR}/cmTestGeneratedHeader.h
  DEPENDS ${CMAKE_COMMAND}
)

#
# Output the files required by 'complex' to a file.
#
# This test has been moved to the 'required' subdir so that it
# has no side-effects on the current Makefile (duplicated source file
# due to source list expansion done twice).
#
SUBDIRS(Temp)

#
# Extra coverage.Not used.
#
INSTALL_TARGETS(/tmp complex)
INSTALL_PROGRAMS(/tmp complex)

CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/Executable/cmVersion.h.in
  ${Complex_BINARY_DIR}/cmVersion.h)

SOURCE_GROUP(A_GROUP ".cxx")
