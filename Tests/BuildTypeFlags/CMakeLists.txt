cmake_minimum_required(VERSION 2.8)

project(BuildTypeFlags C)

enable_testing()
include(CTest)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_C_FLAGS)
set(CMAKE_C_FLAGS_DEBUG
  "${CMAKE_C_FLAGS_DEBUG} -DFLAG=\\\"${CMAKE_BUILD_TYPE}\\\"")
set(CMAKE_C_FLAGS_MINSIZEREL
  "${CMAKE_C_FLAGS_MINSIZEREL} -DFLAG=\\\"${CMAKE_BUILD_TYPE}\\\"")
set(CMAKE_C_FLAGS_RELEASE
  "${CMAKE_C_FLAGS_RELEASE} -DFLAG=\\\"${CMAKE_BUILD_TYPE}\\\"")
set(CMAKE_C_FLAGS_RELWITHDEBINFO
  "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DFLAG=\\\"${CMAKE_BUILD_TYPE}\\\"")

add_library(lib STATIC lib.c)
add_library(lib_shared SHARED lib.c)

add_executable(flags flags.c)
target_link_libraries(flags lib)

add_test(NAME flags_set COMMAND flags)

set_tests_properties(flags_set PROPERTIES
                     PASS_REGULAR_EXPRESSION " ${CMAKE_BUILD_TYPE}")
