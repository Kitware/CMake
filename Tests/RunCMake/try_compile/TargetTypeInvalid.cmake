include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

set(CMAKE_TRY_COMPILE_TARGET_TYPE INVALID)

try_compile(result ${try_compile_bindir_or_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c)
