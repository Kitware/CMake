include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

enable_language(C)

try_compile(RESULT ${try_compile_bindir_or_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  CMAKE_FLAGS           # no values
  COMPILE_DEFINITIONS   # no values
  LINK_LIBRARIES        # no values
  LINK_OPTIONS          # no values
  )
