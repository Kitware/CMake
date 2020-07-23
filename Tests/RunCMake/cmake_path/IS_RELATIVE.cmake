
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

if (WIN32)
  set (path "c:/a")
else()
  set (path "/a")
endif()
cmake_path(IS_RELATIVE path output)
if (output)
  list (APPEND errors "'${path} is relative")
endif()

set (path "a/b")
cmake_path(IS_RELATIVE path output)
if (NOT output)
  list (APPEND errors "'${path} is not relative")
endif()

if (WIN32)
  set (path "c:/a/b")
  cmake_path(IS_RELATIVE path output)
  if (output)
    list (APPEND errors "'${path} is relative")
  endif()

  set (path "//host/b")
  cmake_path(IS_RELATIVE path output)
  if (output)
    list (APPEND errors "'${path} is relative")
  endif()

  set (path "/a")
  cmake_path(IS_RELATIVE path output)
  if (NOT output)
    list (APPEND errors "'${path} is not relative")
  endif()

  set (path "c:a")
  cmake_path(IS_RELATIVE path output)
  if (NOT output)
    list (APPEND errors "'${path} is not relative")
  endif()
endif()


check_errors (IS_RELATIVE ${errors})
