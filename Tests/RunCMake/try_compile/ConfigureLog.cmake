enable_language(C)

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-bad.c
  )

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ConfigureLog-test.c
  )
