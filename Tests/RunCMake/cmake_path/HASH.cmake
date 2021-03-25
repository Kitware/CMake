
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (path1 "a///b/c")
cmake_path(HASH path1 hash1)
set (path2 "a/b////c")
cmake_path(HASH path2 hash2)
if (NOT hash1 STREQUAL hash2)
  list (APPEND errors "'hash values not equal for '${path1}' and '${path2}'")
endif()

set (path1 "a///b/c/../d")
cmake_path(HASH path1 hash1)
set (path2 "a/b////d")
cmake_path(HASH path2 hash2)
if (NOT hash1 STREQUAL hash2)
  list (APPEND errors "'hash values not equal for '${path1}' and '${path2}'")
endif()


check_errors (HASH ${errors})
