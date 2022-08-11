
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (reference "a/./b/..")
cmake_path(NORMAL_PATH reference)
set(output "$<PATH:NORMAL_PATH,a/./b/..>")
if (NOT output STREQUAL reference)
  list (APPEND errors "'${output}' instead of '${reference}'")
endif()

set (reference "a/.///b/../")
cmake_path(NORMAL_PATH reference)
set(output "$<PATH:NORMAL_PATH,a/.///b/../>")
if (NOT output STREQUAL reference)
  list (APPEND errors "'${output}' instead of '${reference}'")
endif()

if (WIN32)
  set (reference "//host/./b/..")
  cmake_path(NORMAL_PATH reference)
  set(output "$<PATH:NORMAL_PATH,//host/./b/..>")
  if (NOT output STREQUAL reference)
    list (APPEND errors "'${output}' instead of '${reference}'")
  endif()

  set (reference "//host/./b/../")
  cmake_path(NORMAL_PATH reference)
  set(output "$<PATH:NORMAL_PATH,//host/./b/../>")
  if (NOT output STREQUAL reference)
    list (APPEND errors "'${output}' instead of '${reference}'")
  endif()

  set (reference "c://a/.///b/../")
  cmake_path(NORMAL_PATH reference)
  set(output "$<PATH:NORMAL_PATH,c://a/.///b/../>")
  if (NOT output STREQUAL reference)
    list (APPEND errors "'${output}' instead of '${reference}'")
  endif()
endif()


check_errors("PATH:NORMAL_PATH" ${errors})
