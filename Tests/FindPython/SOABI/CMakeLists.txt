cmake_minimum_required(VERSION 3.1)

project(TestSOABI C)

find_package(Python3 COMPONENTS ${CMake_TEST_FindPython_COMPONENT})
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Fail to found Python 3")
endif()

if(NOT DEFINED Python3_SOABI)
  message(FATAL_ERROR "Python3_SOABI for ${CMake_TEST_FindPython_COMPONENT} not found")
endif()

if (Python3_Development_FOUND AND Python3_SOABI)
  Python3_add_library (spam3 MODULE WITH_SOABI ../spam.c)
  target_compile_definitions (spam3 PRIVATE PYTHON3)

  get_property (suffix TARGET spam3 PROPERTY SUFFIX)
  if (NOT suffix MATCHES "^.${Python3_SOABI}")
    message(FATAL_ERROR "Module suffix do not include Python3_SOABI")
  endif()
endif()
