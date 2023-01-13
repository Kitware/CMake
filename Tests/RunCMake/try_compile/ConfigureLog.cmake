enable_language(C)

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-bad.c
  LOG_DESCRIPTION "Source that should not compile."
  )

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  NO_LOG
  )

message(CHECK_START "Check 1")
message(CHECK_START "Check 2")
try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  LOG_DESCRIPTION "Source that should compile."
  )
if (COMPILE_RESULT)
  message(CHECK_PASS "passed")
  message(CHECK_PASS "passed")
else()
  message(CHECK_FAIL "failed")
  message(CHECK_FAIL "failed")
endif()
