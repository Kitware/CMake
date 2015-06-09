# a SBCS test case
cmake_minimum_required (VERSION 3.3)
project (SBCS)

add_definitions(-D_SBCS)

add_executable (SBCS SBCS.cxx)
