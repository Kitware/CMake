cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION ASC)

###############################################################################
# Test finding a package that uses the "custom" version schema.
find_package(CustomVersion 42 REQUIRED)
if(NOT CustomVersion_VERSION EQUAL 42)
  message(SEND_ERROR "CustomVersion wrong version ${CustomVersion_VERSION} !")
endif()
