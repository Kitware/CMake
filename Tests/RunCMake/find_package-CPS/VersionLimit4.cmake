cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

###############################################################################
# Test finding a package with multiple suitable versions when a version limit
# is excluding a match.
find_package(Sample 1.2.0...<1.2.3 REQUIRED)
