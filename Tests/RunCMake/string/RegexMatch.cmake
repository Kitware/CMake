
cmake_policy(SET CMP0186 NEW)

function(check_output name expected)
  set(output "${${name}}")
  if(NOT output STREQUAL expected)
    message(FATAL_ERROR "\"string(REGEX)\" set ${name} to \"${output}\", expected \"${expected}\"")
  endif()
endfunction()

string(REGEX MATCH ".*" out "abcd;")
check_output(out "abcd;")

string(REGEX MATCHALL "^.*" out "abcd;")
check_output(out "abcd;")
