
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (path "a///b/c")
cmake_path(IS_PREFIX path "a/b/c/d" output)
if (NOT output)
  list (APPEND errors "'${path} is not prefix of 'a/b/c/d'")
endif()

set (path "a///b/c/../d")
cmake_path(IS_PREFIX path "a/b/d/e" output)
if (output)
  list (APPEND errors "'${path} is prefix of 'a/b/d/e'")
endif()
cmake_path(IS_PREFIX path "a/b/d/e" NORMALIZE output)
if (NOT output)
  list (APPEND errors "'${path} is not prefix of 'a/b/d/e'")
endif()

set(path "/a/b/..")
cmake_path(IS_PREFIX path "/a/c/../b" NORMALIZE output)
if (NOT output)
  list (APPEND errors "'${path} is not prefix of '/a/c/../b'")
endif()

check_errors (IS_PREFIX ${errors})
