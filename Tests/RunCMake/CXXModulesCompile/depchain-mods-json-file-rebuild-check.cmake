file(GLOB synth_dirs
  "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/depchain_with_modules_json_file@synth_*.dir")

list(LENGTH synth_dirs synth_dirs_len)
if (NOT synth_dirs_len EQUAL 0)
  list(APPEND RunCMake_TEST_FAILED
    "Expected no synthetic targets for consuming 'depchain_with_modules_json_file' but found ${synth_dirs_len}: ${synth_dirs}")
endif ()
