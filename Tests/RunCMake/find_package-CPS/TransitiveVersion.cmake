cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION ASC)

###############################################################################
# Test finding a package with a versioned dependency.
find_package(TransitiveVersion REQUIRED)
if(NOT Sample_VERSION STREQUAL "1.2.3+clarke")
  message(SEND_ERROR "Sample wrong version ${Sample_VERSION} !")
endif()
