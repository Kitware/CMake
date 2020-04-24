cmake_minimum_required(VERSION 3.1)

project(TestIronPython2 C)

find_package(Python2 REQUIRED COMPONENTS Interpreter Compiler)
if (NOT Python2_FOUND)
  message (FATAL_ERROR "Fail to found Python 2")
endif()

if (NOT Python2_Compiler_FOUND)
  message (FATAL_ERROR "Fail to found Python 2 Compiler")
endif()
if (NOT Python2_COMPILER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous compiler ID (${Python2_COMPILER_ID})")
endif()

if(NOT TARGET Python2::Interpreter)
  message(SEND_ERROR "Python2::Interpreter not found")
endif()
if(NOT TARGET Python2::Compiler)
  message(SEND_ERROR "Python2::Compiler not found")
endif()
