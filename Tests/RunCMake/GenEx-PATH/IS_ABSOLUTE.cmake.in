
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

if (WIN32)
  set(path "c:/a")
  set(output "$<PATH:IS_ABSOLUTE,c:/a>")
else()
  set(path "/a")
  set(output "$<PATH:IS_ABSOLUTE,/a>")
endif()
if (NOT output)
  list (APPEND errors "'${path}' is not absolute")
endif()

set(output "$<PATH:IS_ABSOLUTE,a/b>")
if (output)
  list (APPEND errors "'a/b' is absolute")
endif()

if (WIN32)
  set(output "$<PATH:IS_ABSOLUTE,c:/a/b>")
  if (NOT output)
    list (APPEND errors "'c:/a/b' is not absolute")
  endif()

  set(output "$<PATH:IS_ABSOLUTE,//host/b>")
  if (NOT output)
    list (APPEND errors "'//host/b' is not absolute")
  endif()

  set(output "$<PATH:IS_ABSOLUTE,/a>")
  if (output)
    list (APPEND errors "'/a' is absolute")
  endif()

  set(output "$<PATH:IS_ABSOLUTE,c:a>")
  if (output)
    list (APPEND errors "'c:a' is absolute")
  endif()
endif()


check_errors("PATH:IS_ABSOLUTE" ${errors})
