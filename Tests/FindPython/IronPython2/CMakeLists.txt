cmake_minimum_required(VERSION 3.5)

project(TestIronPython2 LANGUAGES NONE)

set (Python2_FIND_IMPLEMENTATIONS "IronPython")

find_package(Python2 COMPONENTS Interpreter Compiler)
if (NOT Python2_FOUND)
  message (FATAL_ERROR "Failed to find Python 2")
endif()

if (NOT Python2_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Python 2 Interpreter")
endif()
if (NOT Python2_INTERPRETER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous interpreter ID (${Python2_INTERPRETER_ID})")
endif()

if (NOT Python2_Compiler_FOUND)
  message (FATAL_ERROR "Failed to find Python 2 Compiler")
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
