cmake_minimum_required(VERSION 3.1)

project(TestImplementation${Python_REQUESTED_IMPLEMENTATION} LANGUAGES NONE)


set (Python${Python_REQUESTED_VERSION}_FIND_IMPLEMENTATIONS ${Python_REQUESTED_IMPLEMENTATION})

find_package(Python${Python_REQUESTED_VERSION} COMPONENTS Interpreter)
if (NOT Python${Python_REQUESTED_VERSION}_FOUND OR NOT Python${Python_REQUESTED_VERSION}_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Python${Python_REQUESTED_VERSION}")
endif()

if (Python_REQUESTED_IMPLEMENTATION STREQUAL "IronPython"
    AND NOT Python${Python_REQUESTED_VERSION}_INTERPRETER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous interpreter ID (${Python${Python_REQUESTED_VERSION}_INTERPRETER_ID})")
endif()
if (Python_REQUESTED_IMPLEMENTATION STREQUAL "CPython"
    AND Python${Python_REQUESTED_VERSION}_INTERPRETER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous interpreter ID (${Python${Python_REQUESTED_VERSION}_INTERPRETER_ID})")
endif()


set (Python_FIND_IMPLEMENTATIONS ${Python_REQUESTED_IMPLEMENTATION})

find_package(Python ${Python_REQUESTED_VERSION} REQUIRED COMPONENTS Interpreter)
if (NOT Python_FOUND OR NOT Python_Interpreter_FOUND)
  message (FATAL_ERROR "Failed to find Python ${Python_REQUESTED_VERSION}")
endif()

if (Python_REQUESTED_IMPLEMENTATION STREQUAL "IronPython"
    AND NOT Python_INTERPRETER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous interpreter ID (${Python_INTERPRETER_ID})")
endif()
if (Python_REQUESTED_IMPLEMENTATION STREQUAL "CPython"
    AND Python_INTERPRETER_ID STREQUAL "IronPython")
  message (FATAL_ERROR "Erroneous interpreter ID (${Python_INTERPRETER_ID})")
endif()
