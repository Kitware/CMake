include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(SYCL)

try_compile(result ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src.sycl
  SYCL_CXX_STANDARD 3
  OUTPUT_VARIABLE out
  )

message("try_compile output:\n${out}")
