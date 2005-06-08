CMAKE_MINIMUM_REQUIRED(VERSION 1.3)
#
# Create exe. 
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEST_CXX_FLAGS")

# Link to CMake lib
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source)
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source/kwsys)
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Utilities/cmexpat)
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Utilities/cmzlib)
# Use LINK_LIBRARIES instead of TARGET_LINK_LIBRARIES to 
SET(COMPLEX_LIBS CMakeTestLibrary;CMakeTestLibraryShared;CMakeTestCLibraryShared)
LINK_LIBRARIES(${COMPLEX_LIBS})

ADD_EXECUTABLE(A A.cxx)
ADD_EXECUTABLE(complex complex)
ADD_EXECUTABLE(complex.file complex.file.cxx)
TARGET_LINK_LIBRARIES(complex CMakeLib cmsys cmexpat cmzlib)
IF (UNIX)
  TARGET_LINK_LIBRARIES(complex ${CMAKE_DL_LIBS})
ELSE(UNIX)
  IF (NOT BORLAND)
    IF(NOT MINGW)
      TARGET_LINK_LIBRARIES(complex rpcrt4.lib)
    ENDIF(NOT MINGW)
  ENDIF(NOT BORLAND)
ENDIF (UNIX)

# Test pre-build/pre-link/post-build rules for an executable.
ADD_CUSTOM_COMMAND(TARGET complex PRE_BUILD
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Executable/prebuild.txt")
ADD_CUSTOM_COMMAND(TARGET complex PRE_BUILD
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Executable/prelink.txt")
ADD_CUSTOM_COMMAND(TARGET complex POST_BUILD
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Executable/postbuild.txt")
ADD_CUSTOM_COMMAND(TARGET complex POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                   ARGS -E copy
                        "${Complex_BINARY_DIR}/Executable/postbuild.txt"
                        "${Complex_BINARY_DIR}/Executable/postbuild2.txt")

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
ADD_SUBDIRECTORY(Temp)

#
# Extra coverage.Not used.
#
INSTALL_TARGETS(/tmp complex)
INSTALL_PROGRAMS(/tmp complex)

CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/Executable/cmVersion.h.in
  ${Complex_BINARY_DIR}/cmVersion.h)

SOURCE_GROUP(A_GROUP ".cxx")
SOURCE_GROUP(B_GROUP REGULAR_EXPRESSION "cxx")
SOURCE_GROUP(C_GROUP FILES complex.cxx)

FILE(WRITE ${Complex_BINARY_DIR}/A/libA.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/A/libC.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/B/libB.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/B/libA.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/C/libC.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/C/libB.a "test")
