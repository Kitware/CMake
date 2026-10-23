if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  # AIX shared objects have no soname, so the executable records the
  # name of the file it was linked with.
  execute_process(COMMAND dump -X32_64 -H "${exe}"
    OUTPUT_VARIABLE out RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "dump failed on ${exe}")
  endif()
  string(REPLACE "." "\\." expected_regex "${expected}")
  if(NOT out MATCHES "[ \t]${expected_regex}[ \t]*(\n|$)")
    message(FATAL_ERROR "${exe} does not depend on ${expected}:\n${out}")
  endif()
endif()
