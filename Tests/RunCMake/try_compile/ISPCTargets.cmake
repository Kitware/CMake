include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

enable_language(ISPC)

set(CMAKE_ISPC_INSTRUCTION_SETS avx512knl-i32x16 avx512skx-i32x16)

try_compile(result ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src.ispc
  OUTPUT_VARIABLE out
  )

message("try_compile output:\n${out}")
