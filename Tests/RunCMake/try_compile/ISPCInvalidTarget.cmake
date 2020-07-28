enable_language(ISPC)
set(CMAKE_ISPC_INSTRUCTION_SETS "avxknl-i32x16")
try_compile(result ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src.ispc
  OUTPUT_VARIABLE out
  )
message("try_compile output:\n${out}")
