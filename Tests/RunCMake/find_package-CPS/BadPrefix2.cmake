cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

###############################################################################
# Test that trying to read a .cps whose claimed prefix is longer than its
# actual prefix doesn't crash CMake.
#
# https://gitlab.kitware.com/cmake/cmake/-/issues/27631
find_package(BadPrefix2 REQUIRED)
