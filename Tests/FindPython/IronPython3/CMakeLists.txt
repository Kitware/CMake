cmake_minimum_required(VERSION 3.5)

project(TestIronPython3 LANGUAGES NONE)

set (Python3_FIND_IMPLEMENTATIONS "IronPython")

find_package(Python3 COMPONENTS Interpreter Compiler)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()

if (NOT Python3_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 Interpreter")
endif()
if (NOT Python3_INTERPRETER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous interpreter ID (${Python3_INTERPRETER_ID})")
endif()

if (NOT Python3_Compiler_FOUND)
  message (FATAL_ERROR "Failed to find Python 3 Compiler")
endif()
if (NOT Python3_COMPILER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous compiler ID (${Python3_COMPILER_ID})")
endif()

if(NOT TARGET Python3::Interpreter)
  message(SEND_ERROR "Python3::Interpreter not found")
endif()
if(NOT TARGET Python3::Compiler)
  message(SEND_ERROR "Python3::Compiler not found")
endif()
