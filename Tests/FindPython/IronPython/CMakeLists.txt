cmake_minimum_required(VERSION 3.1)

project(TestIronPython LANGUAGES NONE)

set (Python_FIND_IMPLEMENTATIONS IronPython)

find_package(Python ${Python_REQUESTED_VERSION} COMPONENTS Interpreter Compiler)
if (NOT Python_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}")
endif()

if (NOT Python_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Python Interpreter")
endif()
if (NOT Python_INTERPRETER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous interpreter ID (${Python_INTERPRETER_ID})")
endif()

if (NOT Python_Compiler_FOUND)
  message (FATAL_ERROR "Failed to find Python Compiler")
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
