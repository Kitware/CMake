cmake_minimum_required(VERSION 3.29)
project(${RunCMake_TEST} LANGUAGES C)

# This value is read from the top level CMakeLists.txt
cmake_policy(SET CMP0171 NEW)

include(${RunCMake_TEST}.cmake)
