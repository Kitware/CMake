
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set(output "$<PATH:IS_PREFIX,a///b/c,a/b/c/d>")
if (NOT output)
  list (APPEND errors "'a///b/c' is not prefix of 'a/b/c/d'")
endif()

set(output "$<PATH:IS_PREFIX,a///b/c/../d,a/b/d/e>")
if (output)
  list (APPEND errors "'a///b/c/../d' is prefix of 'a/b/d/e'")
endif()
set(output "$<PATH:IS_PREFIX,NORMALIZE,a///b/c/../d,a/b/d/e>")
if (NOT output)
  list (APPEND errors "'a///b/c/../d' is not prefix of 'a/b/d/e'")
endif()

set(output "$<PATH:IS_PREFIX,NORMALIZE,/a/b/..,/a/c/../b")
if (NOT output)
  list (APPEND errors "'/a/b/..' is not prefix of '/a/c/../b'")
endif()


check_errors("PATH:IS_PREFIX" ${errors})
