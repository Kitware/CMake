#
# Create exe. 
#
ADD_EXECUTABLE(complex complex)

TARGET_LINK_LIBRARIES(complex CMakeTestLibrary)
TARGET_LINK_LIBRARIES(complex CMakeTestLibraryShared)

#
# Link to CMake lib
#
FIND_LIBRARY(CMAKE_LIB
             CMakeLib 
             ${Complex_BINARY_DIR}/../../Source
             ${Complex_BINARY_DIR}/../../Source/Debug
             ${Complex_BINARY_DIR}/../../Source/Release
             ${Complex_BINARY_DIR}/../../Source/MinSizeRel
             ${Complex_BINARY_DIR}/../../Source/RelWithDebInfo)

# Specify the same one for debug/optimized to increase coverage
TARGET_LINK_LIBRARIES(complex 
                      ${CMAKE_LIB}
                      debug ${CMAKE_LIB}
                      optimized ${CMAKE_LIB})

#
# Output the files required by 'complex' to a file.
# The 'complex' executable will then test if this file exists and remove it.
# The contents of this file is not tested (absolute paths).
#
#OUTPUT_REQUIRED_FILES(${Complex_SOURCE_DIR}/Executable/complex.cxx 
#	              ${Complex_BINARY_DIR}/Executable/complex-required.txt)  

#
# Extra coverage.Not used.
#
INSTALL_TARGETS(/tmp complex)
INSTALL_PROGRAMS(/tmp complex)

SOURCE_GROUP(A_GROUP ".cxx")

