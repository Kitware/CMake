if (NOT EXISTS ${ProfilingTestOutput})
  set(RunCMake_TEST_FAILED "Expected ${ProfilingTestOutput} to exists")
endif()

file(READ "${ProfilingTestOutput}" JSON_HEADER LIMIT 2)
if (NOT JSON_HEADER MATCHES "^\\[{")
  set(RunCMake_TEST_FAILED "Expected valid JSON start")
  return()
endif()

file(SIZE "${ProfilingTestOutput}" OUTPUT_SIZE)
math(EXPR END_OFFSET "${OUTPUT_SIZE} -2 ")

file(READ "${ProfilingTestOutput}" JSON_TRAILER OFFSET ${END_OFFSET})
if (NOT JSON_TRAILER MATCHES "^}]$")
  set(RunCMake_TEST_FAILED "Expected valid JSON end")
  return()
endif()
