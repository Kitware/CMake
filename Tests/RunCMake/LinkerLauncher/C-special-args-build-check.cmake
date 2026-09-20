# Launcher invocations cannot be reliably verified via stdout, so it records to
# a log file instead. Verify by checking its content.
set(log_file "${RunCMake_TEST_BINARY_DIR}/special-args.log")
if(NOT EXISTS "${log_file}")
  set(RunCMake_TEST_FAILED "special-args.log was not created by the wrapper")
else()
  file(READ "${log_file}" log_content)
  if(NOT log_content MATCHES "link")
    set(RunCMake_TEST_FAILED
      "special-args.log does not show link launcher ran:\n${log_content}")
  endif()
endif()
