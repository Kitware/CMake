cmake_minimum_required(VERSION 4.3)

project(TestDependencyRunCMake NONE)

if(NOT DEFINED RunCMake_TEST)
  message(FATAL_ERROR "RunCMake_TEST not set")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/${RunCMake_TEST}.cmake")

foreach(target IN ITEMS fake_discovery fake_gtest)
  if(TARGET ${target})
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}-$<CONFIG>.txt"
      CONTENT "$<TARGET_FILE:${target}>")
  endif()
endforeach()
