cmake_minimum_required(VERSION 3.26)
project(GlobalAutogenSystemUseInclude)

include("../AutogenCoreTest.cmake")

block()
  set(test_autogen_use_system_include ON)
  set(CMAKE_AUTOGEN_USE_SYSTEM_INCLUDE ${test_autogen_use_system_include})

  add_executable(autogen_test_on main.cpp)
  get_target_property(target_autogen_use_system_include autogen_test_on AUTOGEN_USE_SYSTEM_INCLUDE)

  if(NOT ${CMAKE_AUTOGEN_USE_SYSTEM_INCLUDE} STREQUAL ${target_autogen_use_system_include})
    message(FATAL_ERROR "CMAKE_AUTOGEN_USE_SYSTEM_INCLUDE not set")
  endif()
endblock()

block()
  set(test_autogen_use_system_include OFF)
  set(CMAKE_AUTOGEN_USE_SYSTEM_INCLUDE ${test_autogen_use_system_include})

  add_executable(autogen_test_off main.cpp)
  get_target_property(target_autogen_use_system_include autogen_test_off AUTOGEN_USE_SYSTEM_INCLUDE)

  if(NOT ${CMAKE_AUTOGEN_USE_SYSTEM_INCLUDE} STREQUAL ${target_autogen_use_system_include})
    message(FATAL_ERROR "CMAKE_AUTOGEN_USE_SYSTEM_INCLUDE not set")
  endif()
endblock()
