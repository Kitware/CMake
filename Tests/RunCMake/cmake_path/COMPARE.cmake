
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set(path "a///b/c")
cmake_path(COMPARE "${path}" EQUAL "a/b/c" output)
if (NOT output)
  list (APPEND errors "'${path}' not equal to 'a/b/c'")
endif()

set (path "a/b/d/../c")
cmake_path(COMPARE "${path}" NOT_EQUAL "a/b/c" output)
if (NOT output)
  list (APPEND errors "'${path}' equal to 'a/b/c'")
endif()
cmake_path(NORMAL_PATH path)
cmake_path(COMPARE "${path}" EQUAL "a/b/c" output)
if (NOT output)
  list (APPEND errors "'${path}' not equal to 'a/b/c'")
endif()

check_errors (COMPARE ${errors})
