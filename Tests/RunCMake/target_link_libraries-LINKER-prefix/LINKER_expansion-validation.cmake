
if (actual_stdout MATCHES "LINKER:" AND NOT linker_prefix_expected)
  set (RunCMake_TEST_FAILED "LINKER: prefix was not expanded.")
  return()
endif()

if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${reference_file}")
  set (RunCMake_TEST_FAILED "${RunCMake_TEST_BINARY_DIR}/${reference_file}: Reference file not found.")
  return()
endif()
file(READ "${RunCMake_TEST_BINARY_DIR}/${reference_file}" linker_flag)

if (NOT actual_stdout MATCHES "${linker_flag}")
  set (RunCMake_TEST_FAILED "LINKER: was not expanded correctly.")
endif()
