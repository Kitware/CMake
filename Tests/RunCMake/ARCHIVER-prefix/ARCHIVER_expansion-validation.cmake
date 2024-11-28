
if (actual_stdout MATCHES "ARCHIVER:" AND NOT archiver_prefix_expected)
  set (RunCMake_TEST_FAILED "ARCHIVER: prefix was not expanded.")
  return()
endif()

if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${reference_file}")
  set (RunCMake_TEST_FAILED "${RunCMake_TEST_BINARY_DIR}/${reference_file}: Reference file not found.")
  return()
endif()
file(READ "${RunCMake_TEST_BINARY_DIR}/${reference_file}" archiver_flag)

if (NOT actual_stdout MATCHES "${archiver_flag}")
  set (RunCMake_TEST_FAILED "ARCHIVER: was not expanded correctly.")
endif()
