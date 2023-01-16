enable_language(C)

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-bad.c
  LOG_DESCRIPTION "Source that should not compile."
  )

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  NO_LOG
  )

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  LOG_DESCRIPTION "Source that should compile."
  )
