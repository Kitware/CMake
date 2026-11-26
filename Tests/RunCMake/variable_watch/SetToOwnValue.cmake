function(watch var access value)
  if(access STREQUAL "MODIFIED_ACCESS")
    message(STATUS "${var} ${access}")
    if(NOT value STREQUAL expected)
      message(SEND_ERROR "${var} is '${value}' instead of '${expected}'")
    endif()
  endif()
endfunction()

set(expected "/a/value/long/enough/to/live/on/the/heap")

set(v1 "${expected}")
variable_watch(v1 watch)
get_property(v1 VARIABLE PROPERTY v1)

set(v2 "${expected}")
variable_watch(v2 watch)
get_directory_property(v2 DEFINITION v2)

cmake_policy(SET CMP0125 OLD)
set(v3 "${expected}")
variable_watch(v3 watch)
find_path(v3 NAMES SetToOwnValue.h NO_CACHE NO_DEFAULT_PATH)
