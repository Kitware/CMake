cmake_minimum_required (VERSION 3.18...3.19)

project (TestVersionRange LANGUAGES C)


find_package (${Python} ${Python_REQUESTED_VERSION} EXACT COMPONENTS Interpreter)
if (NOT ${Python}_FOUND)
  message (FATAL_ERROR "Failed to find ${Python} ${Python_REQUESTED_VERSION}")
endif()

if (Python_REQUESTED_VERSION VERSION_LESS 3.0)
  set (IN_VERSION_RANGE 2.0...<3.0)
  set (OUT_VERSION_RANGE 2.0...<${${Python}_VERSION})
else()
  set (IN_VERSION_RANGE 3.0...<4.0)
  set (OUT_VERSION_RANGE 3.0...<${${Python}_VERSION})
endif()

function (FIND_PYTHON EXPECTED_VERSION RANGE)
  macro (FIND_PYTHON_PACKAGE)
    unset (_${Python}_EXECUTABLE CACHE)
    unset (_${Python}_LIBRARY_RELEASE CACHE)
    unset (_${Python}_INCLUDE_DIR CACHE)
    unset (${Python}_FOUND)

    find_package (${Python} ${ARGV})
  endmacro()

  find_python_package(${RANGE} ${ARGN})

  if (EXPECTED_VERSION STREQUAL "NONE")
    while (${Python}_FOUND AND ${Python}_VERSION VERSION_GREATER ${Python_REQUESTED_VERSION})
      # Possible if multiple versions are installed
      # Try with a different range
      find_python_package(${Python_REQUESTED_VERSION}.0...<${${Python}_VERSION} ${ARGN})
    endwhile()
    if (${Python}_FOUND)
      message (SEND_ERROR "Unexpectedly found version: ${${Python}_VERSION} for '${Python} ${Python_REQUESTED_VERSION}.0...<${${Python}_VERSION} ${ARGN}'")
    endif()
    return()
  endif()

  if (NOT ${Python}_FOUND)
    message (SEND_ERROR "Not found: ${Python} ${RANGE} ${ARGN}")
  elseif (NOT ${Python}_VERSION VERSION_EQUAL EXPECTED_VERSION)
    message (SEND_ERROR "Wrong version: ${${Python}_VERSION} for '${Python} ${RANGE} ${ARGN}'")
  endif()
endfunction()

find_python (${${Python}_VERSION} ${IN_VERSION_RANGE} COMPONENTS Interpreter)
if (${Python}_FIND_IMPLEMENTATIONS STREQUAL "IronPython")
  find_python (${${Python}_VERSION} ${IN_VERSION_RANGE} COMPONENTS Compiler)
else()
  find_python (${${Python}_VERSION} ${IN_VERSION_RANGE} COMPONENTS Development)
endif()

find_python ("NONE" ${OUT_VERSION_RANGE} COMPONENTS Interpreter)
if (${Python}_FIND_IMPLEMENTATIONS STREQUAL "IronPython")
  find_python ("NONE" ${OUT_VERSION_RANGE} COMPONENTS Compiler)
else()
  find_python ("NONE" ${OUT_VERSION_RANGE} COMPONENTS Development)
endif()

find_python ("NONE" 5...6 COMPONENTS Interpreter)
