cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

###############################################################################
# Test finding a package that is missing dependencies.
find_package(Incomplete REQUIRED)
