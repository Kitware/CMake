execute_process(COMMAND ${CMAKE_COMMAND} -P ${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake
  OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT out MATCHES "-- Install configuration: .*-- codegenexlib")
  string(REGEX REPLACE "\n" "\n  " out "  ${out}")
  string(APPEND RunCMake_TEST_FAILED
      "\"-- codegenexlib\" was not found:\n${out}")
endif()
