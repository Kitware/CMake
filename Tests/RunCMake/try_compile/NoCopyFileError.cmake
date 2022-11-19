include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})
try_compile(RESULT ${try_compile_bindir_or_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/copied.bin COPY_FILE_ERROR)
