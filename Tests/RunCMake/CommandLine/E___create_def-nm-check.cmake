string(REGEX REPLACE [[^E___create_def-nm-]] "" subtest "${test}")
cmake_path(APPEND RunCMake_TEST_BINARY_DIR "${subtest}.def" OUTPUT_VARIABLE actual_def)
cmake_path(APPEND RunCMake_TEST_SOURCE_DIR "${test}-expected.def" OUTPUT_VARIABLE expected_def)
execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files --ignore-eol ${actual_def} ${expected_def}
  RESULT_VARIABLE compare_result)
if(compare_result)
  set(RunCMake_TEST_FAILED "${actual_def} does not match ${expected_def}")
endif()
