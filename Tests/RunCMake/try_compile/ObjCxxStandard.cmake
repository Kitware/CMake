enable_language(OBJCXX)
try_compile(result ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src.mm
  OBJCXX_STANDARD 3
  OUTPUT_VARIABLE out
  )
message("try_compile output:\n${out}")
