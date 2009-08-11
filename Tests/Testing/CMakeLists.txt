#
# Testing
#
cmake_minimum_required (VERSION 2.7)
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
ADD_TEST(testing.1 ${Testing_BINARY_DIR}/bin/testing)

#
# skip level test
#
ADD_SUBDIRECTORY(Sub/Sub2)

# Per-config target name test.
ADD_LIBRARY(pcStatic STATIC pcStatic.c)
SET_PROPERTY(TARGET pcStatic PROPERTY RELEASE_POSTFIX -opt)
SET_PROPERTY(TARGET pcStatic PROPERTY DEBUG_POSTFIX -dbg)
ADD_LIBRARY(pcShared SHARED pcShared.c)
SET_PROPERTY(TARGET pcShared PROPERTY RELEASE_POSTFIX -opt)
SET_PROPERTY(TARGET pcShared PROPERTY DEBUG_POSTFIX -dbg)
SET_PROPERTY(TARGET pcShared PROPERTY VERSION 1.2)
SET_PROPERTY(TARGET pcShared PROPERTY SOVERSION 3)
IF(NOT WIN32)
  SET(soname_file -DpcShared_soname_file=$<TARGET_SONAME_FILE:pcShared>)
ENDIF()
ADD_EXECUTABLE(perconfig perconfig.c)
TARGET_LINK_LIBRARIES(perconfig pcStatic pcShared)
SET_PROPERTY(TARGET perconfig PROPERTY RELEASE_POSTFIX -opt)
SET_PROPERTY(TARGET perconfig PROPERTY DEBUG_POSTFIX -dbg)
ADD_TEST(NAME testing.perconfig COMMAND perconfig)

# Test using a driver script with generator expressions.
ADD_TEST(NAME testing.driver
  COMMAND ${CMAKE_COMMAND}
    -Dconfiguration=$<CONFIGURATION>
    -Dperconfig_file_dir=$<TARGET_FILE_DIR:perconfig>
    -Dperconfig_file_name=$<TARGET_FILE_NAME:perconfig>
    -Dperconfig_file=$<TARGET_FILE:perconfig>
    -DpcStatic_file=$<TARGET_FILE:pcStatic>
    -DpcStatic_linker_file=$<TARGET_LINKER_FILE:pcStatic>
    -DpcShared_file=$<TARGET_FILE:pcShared>
    -DpcShared_linker_file=$<TARGET_LINKER_FILE:pcShared>
    ${soname_file}
    -P ${Testing_SOURCE_DIR}/driver.cmake
  )
