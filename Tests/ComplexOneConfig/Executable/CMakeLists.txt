#
# Create exe. 
#
SET_SOURCE_FILES_PROPERTIES(complex COMPILE_FLAGS "-DSET_SOURCE_FILES_PROPERTIES")
ADD_EXECUTABLE(complex complex)
SET(COMPLEX_LIBS CMakeTestLibrary;CMakeTestLibraryShared)
TARGET_LINK_LIBRARIES(complex ${COMPLEX_LIBS})

#
# Link to CMake lib
# Specify the same one for debug/optimized to increase coverage
#
LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source)

TARGET_LINK_LIBRARIES(complex 
                      CMakeLib
                      debug CMakeLib
                      optimized CMakeLib)

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

SOURCE_GROUP(A_GROUP ".cxx")
