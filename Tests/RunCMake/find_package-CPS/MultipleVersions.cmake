cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

###############################################################################
# Test finding a package with multiple suitable versions (permissive match).
find_package(Sample 1.1.0 REQUIRED)
if(NOT Sample_VERSION STREQUAL "1.2.3+clarke")
  message(SEND_ERROR "Sample wrong version ${Sample_VERSION} !")
endif()
