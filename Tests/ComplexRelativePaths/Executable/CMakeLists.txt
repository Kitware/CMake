CMAKE_MINIMUM_REQUIRED(VERSION 1.3)
#
# Create exe. 
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEST_CXX_FLAGS")
SET_SOURCE_FILES_PROPERTIES(complex COMPILE_FLAGS 
                            "-DFILE_HAS_EXTRA_COMPILE_FLAGS")
# Link to CMake lib
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source)
# Use LINK_LIBRARIES instead of TARGET_LINK_LIBRARIES to 
SET(COMPLEX_LIBS CMakeTestLibrary;CMakeTestLibraryShared;CMakeTestCLibraryShared)
LINK_LIBRARIES(${COMPLEX_LIBS})


ADD_EXECUTABLE(complex complex)
TARGET_LINK_LIBRARIES(complex 
                      CMakeLib)

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
