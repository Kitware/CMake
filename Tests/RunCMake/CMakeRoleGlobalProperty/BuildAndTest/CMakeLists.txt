cmake_minimum_required(VERSION 3.12)
project(CMakeRoleGlobalPropertyBuildAndTest NONE)
include(CTest)

get_property(role GLOBAL PROPERTY CMAKE_ROLE)
if(NOT role STREQUAL "PROJECT")
  message(SEND_ERROR "CMAKE_ROLE property is \"${role}\", should be \"PROJECT\"")
endif()

add_test(NAME RunCMakeVersion COMMAND "${CMAKE_COMMAND}" --version)
