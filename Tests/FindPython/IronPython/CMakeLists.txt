cmake_minimum_required(VERSION 3.1)

project(TestIronPython C)

find_package(Python ${Python_REQUESTED_VERSION} REQUIRED COMPONENTS Interpreter Compiler)
if (NOT Python_FOUND)
  message (FATAL_ERROR "Fail to found Python ${Python_REQUESTED_VERSION}")
endif()

if (NOT Python_Compiler_FOUND)
  message (FATAL_ERROR "Fail to found Python Compiler")
endif()
if (NOT Python_COMPILER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous compiler ID (${Python_COMPILER_ID})")
endif()

if(NOT TARGET Python::Interpreter)
  message(SEND_ERROR "Python::Interpreter not found")
endif()
if(NOT TARGET Python::Compiler)
  message(SEND_ERROR "Python::Interpreter not found")
endif()
