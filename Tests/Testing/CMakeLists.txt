#
# Testing
#
cmake_minimum_required (VERSION 2.6)
PROJECT (Testing)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
     ${Testing_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${Testing_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all executables.")

#
# Where will executable tests be written ?
#
IF (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE (EXECUTABLE_OUTPUT_PATH)
  SET (CXX_TEST_PATH .)
ENDIF (EXECUTABLE_OUTPUT_PATH)

#
# Include Dart
# (will also set NSLOOKUP, HOSTNAME, etc.)
#
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

#
# Extra coverage
#
BUILD_COMMAND(BUILD_COMMAND_VAR ${CMAKE_MAKE_PROGRAM})
BUILD_NAME(BUILD_NAME_VAR)
SITE_NAME(SITE_NAME_VAR)

#
# Enable testing
#
ENABLE_TESTING()

#
# Add test
#
ADD_EXECUTABLE(testing testing.cxx)
ADD_TEST(testing ${Testing_BINARY_DIR}/bin/testing)

#
# skip level test
#
ADD_SUBDIRECTORY(Sub/Sub2)
