enable_language(C)


find_package (${PYTHON} ${Python_REQUESTED_VERSION} EXACT COMPONENTS Interpreter)
if (NOT ${PYTHON}_FOUND)
  message (FATAL_ERROR "Failed to find ${PYTHON} ${Python_REQUESTED_VERSION}")
endif()

if (Python_REQUESTED_VERSION VERSION_LESS 3.0)
  set (IN_VERSION_RANGE 2.0...<3.0)
  set (OUT_VERSION_RANGE 2.0...<${${PYTHON}_VERSION})
else()
  set (IN_VERSION_RANGE 3.0...<4.0)
  set (OUT_VERSION_RANGE 3.0...<${${PYTHON}_VERSION})
endif()

function (FIND_PYTHON EXPECTED_VERSION RANGE)
  macro (FIND_PYTHON_PACKAGE)
    unset (_${PYTHON}_EXECUTABLE CACHE)
    unset (_${PYTHON}_LIBRARY_RELEASE CACHE)
    unset (_${PYTHON}_INCLUDE_DIR CACHE)
    unset (${PYTHON}_FOUND)

    find_package (${PYTHON} ${ARGV})
  endmacro()

  find_python_package(${RANGE} ${ARGN})

  if (EXPECTED_VERSION STREQUAL "NONE")
    while (${PYTHON}_FOUND AND ${PYTHON}_VERSION VERSION_GREATER ${Python_REQUESTED_VERSION})
      # Possible if multiple versions are installed
      # Try with a different range
      find_python_package(${Python_REQUESTED_VERSION}.0...<${${PYTHON}_VERSION} ${ARGN})
    endwhile()
    if (${PYTHON}_FOUND)
      message (SEND_ERROR "Unexpectedly found version: ${${PYTHON}_VERSION} for '${PYTHON} ${Python_REQUESTED_VERSION}.0...<${${PYTHON}_VERSION} ${ARGN}'")
    endif()
    return()
  endif()

  if (NOT ${PYTHON}_FOUND)
    message (SEND_ERROR "Not found: ${PYTHON} ${RANGE} ${ARGN}")
  elseif (NOT ${PYTHON}_VERSION VERSION_EQUAL EXPECTED_VERSION)
    message (SEND_ERROR "Wrong version: ${${PYTHON}_VERSION} for '${PYTHON} ${RANGE} ${ARGN}'")
  endif()
endfunction()

find_python (${${PYTHON}_VERSION} ${IN_VERSION_RANGE} COMPONENTS Interpreter)
if (${PYTHON}_FIND_IMPLEMENTATIONS STREQUAL "IronPython")
  find_python (${${PYTHON}_VERSION} ${IN_VERSION_RANGE} COMPONENTS Compiler)
else()
  find_python (${${PYTHON}_VERSION} ${IN_VERSION_RANGE} COMPONENTS Development)
endif()

find_python ("NONE" ${OUT_VERSION_RANGE} COMPONENTS Interpreter)
if (${PYTHON}_FIND_IMPLEMENTATIONS STREQUAL "IronPython")
  find_python ("NONE" ${OUT_VERSION_RANGE} COMPONENTS Compiler)
else()
  find_python ("NONE" ${OUT_VERSION_RANGE} COMPONENTS Development)
endif()

find_python ("NONE" 5...6 COMPONENTS Interpreter)
