try_run(RUN_RESULT COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-bad.c
  LOG_DESCRIPTION "Source that should not compile."
  )

try_run(RUN_RESULT COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  NO_LOG
  )

message(CHECK_START "Check 1")
try_run(RUN_RESULT COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  LOG_DESCRIPTION "Source that should compile."
  )

message(CHECK_START "Check 2")
try_run(RUN_RESULT COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  RUN_OUTPUT_VARIABLE RUN_OUTPUT
  )
if (RUN_RESULT)
  message(CHECK_PASS "passed")
  message(CHECK_PASS "passed")
else()
  message(CHECK_FAIL "failed")
  message(CHECK_FAIL "failed")
endif()

try_run(RUN_RESULT COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  RUN_OUTPUT_STDOUT_VARIABLE RUN_STDOUT
  RUN_OUTPUT_STDERR_VARIABLE RUN_STDERR
  )
