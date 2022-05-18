
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (reference "../../a/d")
cmake_path(ABSOLUTE_PATH reference BASE_DIRECTORY "/x/y/a/f")
set(output "$<PATH:ABSOLUTE_PATH,../../a/d,/x/y/a/f>")
if (NOT output STREQUAL reference)
  list (APPEND errors "'${output}' instead of '${reference}'")
endif()

set (reference "../../a/d")
cmake_path(ABSOLUTE_PATH reference BASE_DIRECTORY "/x/y/a/f" NORMALIZE)
set(output "$<PATH:ABSOLUTE_PATH,NORMALIZE,../../a/d,/x/y/a/f>")
if (NOT output STREQUAL reference)
  list (APPEND errors "'${output}' instead of '${reference}'")
endif()

set (reference "/a/d/../e")
cmake_path(ABSOLUTE_PATH reference BASE_DIRECTORY "/x/y/a/f")
set(output "$<PATH:ABSOLUTE_PATH,/a/d/../e,/x/y/a/f>")
if (NOT output STREQUAL reference)
  list (APPEND errors "'${output}' instead of '${reference}'")
endif()

set (reference "/a/d/../e")
cmake_path(ABSOLUTE_PATH reference BASE_DIRECTORY "/x/y/a/f" NORMALIZE)
set(output "$<PATH:ABSOLUTE_PATH,NORMALIZE,/a/d/../e,/x/y/a/f>")
if (NOT output STREQUAL reference)
  list (APPEND errors "'${output}' instead of '${reference}'")
endif()


check_errors("PATH:ABSOLUTE_PATH" ${errors})
