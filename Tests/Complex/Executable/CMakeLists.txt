ADD_EXECUTABLE(complex complex)

TARGET_LINK_LIBRARIES(complex CMakeTestLibrary)
TARGET_LINK_LIBRARIES(complex CMakeTestLibraryShared)

FIND_LIBRARY(CMAKE_LIB
             CMakeLib 
             ${Complex_BINARY_DIR}/../../Source
             ${Complex_BINARY_DIR}/../../Source/Debug
             ${Complex_BINARY_DIR}/../../Source/Release
             ${Complex_BINARY_DIR}/../../Source/MinSizeRel
             ${Complex_BINARY_DIR}/../../Source/RelWithDebInfo)

TARGET_LINK_LIBRARIES(complex ${CMAKE_LIB})
LINK_LIBRARIES(${CMAKE_LIB})

#
# Testing
#
ADD_TEST(complex ${Complex_BINARY_DIR}/bin/complex)

#
# More coverage
#
INSTALL_TARGETS(/tmp complex)
INSTALL_PROGRAMS(/tmp complex)
