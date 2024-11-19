cmake_minimum_required(VERSION 3.31)
project(FindPackageCpsTest)

set(CMAKE_EXPERIMENTAL_FIND_CPS_PACKAGES "e82e467b-f997-4464-8ace-b00808fff261")

# Protect tests from running inside the default install prefix.
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/NotDefaultPrefix")

# Disable built-in search paths.
set(CMAKE_FIND_USE_PACKAGE_ROOT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH OFF)
set(CMAKE_FIND_USE_INSTALL_PREFIX OFF)

# Enable framework searching.
set(CMAKE_FIND_FRAMEWORK FIRST)

add_executable(FindPackageCpsTest FindPackageTest.cxx)

###############################################################################
# Test a basic package search.
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Sample CONFIG)
if(NOT Sample_FOUND)
  message(SEND_ERROR "Sample not found !")
elseif(NOT Sample_VERSION STREQUAL "2.10.11")
  message(SEND_ERROR "Sample wrong version ${Sample_VERSION} !")
elseif(NOT Sample_VERSION_MAJOR EQUAL 2)
  message(SEND_ERROR "Sample wrong major version ${Sample_VERSION_MAJOR} !")
elseif(NOT Sample_VERSION_MINOR EQUAL 10)
  message(SEND_ERROR "Sample wrong minor version ${Sample_VERSION_MINOR} !")
elseif(NOT Sample_VERSION_PATCH EQUAL 11)
  message(SEND_ERROR "Sample wrong patch version ${Sample_VERSION_PATCH} !")
elseif(NOT Sample_VERSION_TWEAK EQUAL 0)
  message(SEND_ERROR "Sample wrong tweak version ${Sample_VERSION_TWEAK} !")
endif()

set(CMAKE_PREFIX_PATH)
