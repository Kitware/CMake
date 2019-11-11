enable_language(OBJC)
try_compile(result ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src.m
  OBJC_STANDARD 3
  OUTPUT_VARIABLE out
  )
message("try_compile output:\n${out}")
