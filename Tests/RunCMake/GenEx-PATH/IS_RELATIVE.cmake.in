
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)


if (WIN32)
  set(path "c:/a")
  set(output "$<PATH:IS_RELATIVE,c:/a>")
else()
  set(path "/a")
  set(output "$<PATH:IS_RELATIVE,/a>")
endif()
if (output)
  list (APPEND errors "'${path} is relative")
endif()

set(output "$<PATH:IS_RELATIVE,a/b>")
if (NOT output)
  list (APPEND errors "'a/b' is not relative")
endif()

if (WIN32)
  set(output "$<PATH:IS_RELATIVE,c:/a/b>")
  if (output)
    list (APPEND errors "'c:/a/b' is relative")
  endif()

  set(output "$<PATH:IS_RELATIVE,//host/b>")
  if (output)
    list (APPEND errors "'//host/b' is relative")
  endif()

  set(output "$<PATH:IS_RELATIVE,/a>")
  if (NOT output)
    list (APPEND errors "'/a' is not relative")
  endif()

  set(output "$<PATH:IS_RELATIVE,c:a>")
  if (NOT output)
    list (APPEND errors "'c:a' is not relative")
  endif()
endif()


check_errors("PATH:IS_RELATIVE" ${errors})
