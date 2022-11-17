include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

enable_language(C)

try_compile(result ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  C_STANDARD 3 # bogus, but not used
  OUTPUT_VARIABLE out
  )

if(NOT result)
  message(FATAL_ERROR "try_compile failed:\n${out}")
endif()
