cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

###############################################################################
# Test reporting when trying to read a .cps whose absolute prefix cannot be
# determined.
find_package(BadPrefix REQUIRED)
