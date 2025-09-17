include(${RunCMake_SOURCE_DIR}/TARGETS-FILE_RPATH_CHANGE-check-common.cmake)
skip_without_rpath_change_rule()
string(APPEND prefix "${wsnl}" [[FILE "[^"]*/]])
string(REGEX QUOTE RunCMake_BINARY_DIR_regex "${RunCMake_BINARY_DIR}")

set(target "exe1")
string(CONCAT regex "${prefix}${target}\"${wsnl}"
              [[OLD_RPATH "]] "${RunCMake_BINARY_DIR_regex}")
check()

if("x${CMAKE_SHARED_LIBRARY_RPATH_ORIGIN_TOKEN}" STREQUAL "x\$ORIGIN")
  set(target "exe2")
  string(CONCAT regex "${prefix}${target}\"${wsnl}"
                [[OLD_RPATH "\\\$ORIGIN]])
  check()
endif()
