cmake_minimum_required(VERSION 3.14)
project(embt_compiler_id C)
# The parent test reads this line back to assert the detected identity.
message(STATUS "DRIVER_COMPILER_ID=[${CMAKE_C_COMPILER_ID}]")
