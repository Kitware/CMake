
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (path "a/b/c.e.f")
cmake_path (REMOVE_FILENAME path)
if (NOT path STREQUAL "a/b/")
  list (APPEND errors "'${path}' instead of 'a/b/'")
endif()

cmake_path (REMOVE_FILENAME path)
if (NOT path STREQUAL "a/b/")
  list (APPEND errors "'${path}' instead of 'a/b/'")
endif()

set (path "a/b/c.e.f")
cmake_path (REMOVE_FILENAME path OUTPUT_VARIABLE output)
if (NOT path STREQUAL "a/b/c.e.f")
  list (APPEND errors "input changed unexpectedly")
endif()
if (NOT output STREQUAL "a/b/")
  list (APPEND errors "'${output}' instead of 'a/b/'")
endif()

check_errors (REMOVE_FILENAME ${errors})
