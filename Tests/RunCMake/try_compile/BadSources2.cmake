include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})
enable_language(C)
try_compile(RESULT ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src.c
                                   ${CMAKE_CURRENT_SOURCE_DIR}/src.cxx
  )
