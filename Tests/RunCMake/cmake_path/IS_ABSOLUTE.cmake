
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

if (WIN32)
  set (path "c:/a")
else()
  set (path "/a")
endif()
cmake_path(IS_ABSOLUTE path output)
if (NOT output)
  list (APPEND errors "'${path} is not absolute")
endif()

set (path "a/b")
cmake_path(IS_ABSOLUTE path output)
if (output)
  list (APPEND errors "'${path} is absolute")
endif()

if (WIN32)
  set (path "c:/a/b")
  cmake_path(IS_ABSOLUTE path output)
  if (NOT output)
    list (APPEND errors "'${path} is not absolute")
  endif()

  set (path "//host/b")
  cmake_path(IS_ABSOLUTE path output)
  if (NOT output)
    list (APPEND errors "'${path} is not absolute")
  endif()

  set (path "/a")
  cmake_path(IS_ABSOLUTE path output)
  if (output)
    list (APPEND errors "'${path} is absolute")
  endif()

  set (path "c:a")
  cmake_path(IS_ABSOLUTE path output)
  if (output)
    list (APPEND errors "'${path} is absolute")
  endif()
endif()


check_errors (IS_ABSOLUTE ${errors})
