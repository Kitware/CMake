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

TARGET_LINK_LIBRARIES(complex ${CMAKE_LIB})

#
# Extra coverage.Not used.
#
INSTALL_TARGETS(/tmp complex)
INSTALL_PROGRAMS(/tmp complex)

SOURCE_GROUP(A_GROUP ".cxx")
