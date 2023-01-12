include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

# Pretend we are cross-compiling to take that try_run code path.
set(CMAKE_CROSSCOMPILING 1)
set(RUN_RESULT 0)
try_run(RUN_RESULT COMPILE_RESULT ${try_compile_bindir_or_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c)
unset(CMAKE_CROSSCOMPILING)
