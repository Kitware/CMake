
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (path "/a/b")
cmake_path (APPEND_STRING path "cd")
if (NOT path STREQUAL "/a/bcd")
  list (APPEND errors "'${path}' instead of 'a/bcd'")
endif()

set (path "/a/b")
cmake_path (APPEND_STRING path "cd" "ef" OUTPUT_VARIABLE output)
if (NOT path STREQUAL "/a/b")
  list (APPEND errors "input changed unexpectedly")
endif()
if (NOT output STREQUAL "/a/bcdef")
  list (APPEND errors "'${output}' instead of 'a/bcdef'")
endif()

check_errors (APPEND_STRING ${errors})
