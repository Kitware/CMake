cmake_minimum_required(VERSION 3.24)
project(VerifyHeaderSet LANGUAGES C)

message(STATUS "Inside subproject, var = '${CMAKE_VERIFY_INTERFACE_HEADER_SETS}'")

add_library(Blah INTERFACE)
target_sources(Blah
  INTERFACE FILE_SET HEADERS FILES blah.h
)
