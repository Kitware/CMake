cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

###############################################################################
# Test finding a package with multiple suitable versions when a version limit
# is present and restricting the matches.
find_package(Sample 1.1.0...<1.2.0 REQUIRED)
if(NOT Sample_VERSION STREQUAL "1.1.0+asimov")
  message(SEND_ERROR "Sample wrong version ${Sample_VERSION} !")
endif()
