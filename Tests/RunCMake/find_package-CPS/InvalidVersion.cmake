cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION ASC)

###############################################################################
# Test that malformed versions are rejected (rather than crashing).
find_package(InvalidVersion 1.0 QUIET)
if(InvalidVersion_FOUND)
  message(SEND_ERROR "InvalidVersion unexpectedly found with version "
                     "\"${InvalidVersion_VERSION}\" !")
endif()
