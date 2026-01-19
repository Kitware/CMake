cmake_minimum_required(VERSION 3.24)
project(VerifyHeaderSet LANGUAGES C)

message(STATUS "Inside subproject, interface var = '${CMAKE_VERIFY_INTERFACE_HEADER_SETS}'")
message(STATUS "Inside subproject, private var = '${CMAKE_VERIFY_PRIVATE_HEADER_SETS}'")

add_library(Blah INTERFACE)
target_sources(Blah
  PUBLIC FILE_SET HEADERS FILES blah.h
)
