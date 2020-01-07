cmake_minimum_required(VERSION 3.1)

project(TestSOABI C)

find_package(Python3 COMPONENTS ${CMake_TEST_FindPython_COMPONENT})
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Fail to found Python 3")
endif()

if(NOT DEFINED Python3_SOABI)
  message(FATAL_ERROR "Python3_SOABI for ${CMake_TEST_FindPython_COMPONENT} not found")
endif()
