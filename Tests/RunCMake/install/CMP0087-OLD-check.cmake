execute_process(COMMAND ${CMAKE_COMMAND} -P ${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake
  OUTPUT_VARIABLE out ERROR_VARIABLE err)

if(NOT out MATCHES "-- Install configuration: .*-- \\$<TARGET_PROPERTY:codegenexlib,NAME>")
  string(REGEX REPLACE "\n" "\n  " out "  ${out}")
  string(APPEND RunCMake_TEST_FAILED
      "\"-- $<TARGET_PROPERTY:codegenexlib,NAME>\" was not found:\n${out}")
endif()
