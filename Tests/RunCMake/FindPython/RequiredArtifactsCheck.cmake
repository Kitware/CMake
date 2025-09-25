enable_language(C)

include("${PYTHON_ARTIFACTS}")

set (components)
if (CHECK_INTERPRETER)
  set (required_interpreter "${Python3_EXECUTABLE}")
  list (APPEND components Interpreter)
endif()
if (CHECK_LIBRARY OR CHECK_INCLUDE)
  list (APPEND components Development)
  if (CHECK_LIBRARY)
    set (required_library "${Python3_LIBRARY}")
  endif()
  if (CHECK_INCLUDE)
    set (required_include "${Python3_INCLUDE_DIR}")
  endif()
endif()
if (CHECK_SABI_LIBRARY)
  list (APPEND components Development.SABIModule)
  set (required_sabi_library "${Python3_SABI_LIBRARY}")
endif()

find_package (Python3 COMPONENTS ${components})


if (PYTHON_IS_FOUND AND NOT Python3_FOUND)
  message (FATAL_ERROR "Python3 unexpectedly not found")
endif()
if (NOT PYTHON_IS_FOUND AND Python3_FOUND)
  message (FATAL_ERROR "Python3 unexpectedly found")
endif()

if (CHECK_INTERPRETER AND NOT Python3_EXECUTABLE STREQUAL required_interpreter)
  message (FATAL_ERROR "Failed to use input variable Python3_EXECUTABLE")
endif()

if (CHECK_LIBRARY AND PYTHON_IS_FOUND AND
    (NOT Python3_LIBRARY_RELEASE STREQUAL PYTHON_LIBRARY_RELEASE
      OR (WIN32 AND NOT Python3_RUNTIME_LIBRARY_RELEASE STREQUAL PYTHON_RUNTIME_LIBRARY_RELEASE)
      OR (PYTHON_LIBRARY_DEBUG AND NOT Python3_LIBRARY_DEBUG STREQUAL PYTHON_LIBRARY_DEBUG)
      OR (WIN32 AND PYTHON_RUNTIME_LIBRARY_DEBUG AND NOT Python3_RUNTIME_LIBRARY_DEBUG STREQUAL PYTHON_RUNTIME_LIBRARY_DEBUG)))
  message (FATAL_ERROR "Failed to use input variable Python3_LIBRARY")
endif()
if (CHECK_LIBRARY AND NOT PYTHON_IS_FOUND AND
    NOT Python3_LIBRARY_RELEASE STREQUAL required_library)
  message (FATAL_ERROR "Failed to use input variable Python3_LIBRARY")
endif()

if (CHECK_INCLUDE AND NOT Python3_INCLUDE_DIRS STREQUAL required_include)
  message (FATAL_ERROR "Failed to use input variable Python3_INCLUDE_DIR")
endif()

if (CHECK_SABI_LIBRARY AND PYTHON_IS_FOUND AND
    (NOT Python3_SABI_LIBRARY_RELEASE STREQUAL PYTHON_SABI_LIBRARY_RELEASE
      OR (WIN32 AND NOT Python3_RUNTIME_SABI_LIBRARY_RELEASE STREQUAL PYTHON_RUNTIME_SABI_LIBRARY_RELEASE)
      OR (PYTHON_SABI_LIBRARY_DEBUG AND NOT Python3_SABI_LIBRARY_DEBUG STREQUAL PYTHON_SABI_LIBRARY_DEBUG)
      OR (WIN32 AND PYTHON_RUNTIME_SABI_LIBRARY_DEBUG AND NOT Python3_RUNTIME_SABI_LIBRARY_DEBUG STREQUAL PYTHON_RUNTIME_SABI_LIBRARY_DEBUG)))
  message (FATAL_ERROR "Failed to use input variable Python3_SABI_LIBRARY")
endif()
if (CHECK_SABI_LIBRARY AND NOT PYTHON_IS_FOUND AND
    NOT Python3_SABI_LIBRARY_RELEASE STREQUAL required_sabi_library)
    message (FATAL_ERROR "Failed to use input variable Python3_SABI_LIBRARY")
endif()
