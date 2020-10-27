
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (path "../../a/d")
cmake_path(ABSOLUTE_PATH path BASE_DIRECTORY "/x/y/a/f")
if (NOT path STREQUAL "/x/y/a/f/../../a/d")
  list (APPEND errors "'${path}' instead of '/x/y/a/f/../../a/d'")
endif()

set (path "../../a/d")
cmake_path(ABSOLUTE_PATH path BASE_DIRECTORY "/x/y/a/f" NORMALIZE)
if (NOT path STREQUAL "/x/y/a/d")
  list (APPEND errors "'${path}' instead of '/x/y/a/d'")
endif()

set (path "../../a/d")
cmake_path(ABSOLUTE_PATH path BASE_DIRECTORY "/x/y/a/f" NORMALIZE OUTPUT_VARIABLE output)
if (NOT path STREQUAL "../../a/d")
  list (APPEND errors "input changed unexpectedly")
endif()
if (NOT output STREQUAL "/x/y/a/d")
  list (APPEND errors "'${output}' instead of '/x/y/a/d'")
endif()

set (path "/a/d/../e")
cmake_path(ABSOLUTE_PATH path BASE_DIRECTORY "/x/y/a/f")
if (NOT path STREQUAL "/a/d/../e")
  list (APPEND errors "'${path}' instead of '/a/d/../e'")
endif()

set (path "/a/d/../e")
cmake_path(ABSOLUTE_PATH path BASE_DIRECTORY "/x/y/a/f" NORMALIZE)
if (NOT path STREQUAL "/a/e")
  list (APPEND errors "'${path}' instead of '/a/e'")
endif()


check_errors (ABSOLUTE_PATH ${errors})
