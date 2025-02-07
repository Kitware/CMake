cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

###############################################################################
# Test requesting components with missing dependencies from a package.
find_package(ComponentTest REQUIRED COMPONENTS Incomplete)
