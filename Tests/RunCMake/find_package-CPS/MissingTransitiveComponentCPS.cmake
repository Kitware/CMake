cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

###############################################################################
# Test depending on components of another package which are unavailable.
find_package(TransitiveMissingCPS REQUIRED)
