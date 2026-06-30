cmake_minimum_required(VERSION 3.10)
if(NOT RunCMake_TEST)
  set(RunCMake_TEST "$ENV{RunCMake_TEST}") # needed when cache is deleted
endif()
if(RunCMake_TEST MATCHES "^ToolchainFromFile")
  # CMAKE_TOOLCHAIN_FILE should be set before project. The ToolchainFromFile
  # cases test detecting toolchain path changes between CMake invocations and
  # not mid-configure.
  file(READ "${CMAKE_BINARY_DIR}/toolchain.txt" toolchain_file)
  string(STRIP ${toolchain_file} toolchain_file)
  set(CMAKE_TOOLCHAIN_FILE "${toolchain_file}")
endif()
project(${RunCMake_TEST} NONE)
include(${RunCMake_TEST}.cmake)
