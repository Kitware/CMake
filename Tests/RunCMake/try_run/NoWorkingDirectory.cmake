include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

try_run(RUN_RESULT COMPILE_RESULT ${try_compile_bindir_or_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  WORKING_DIRECTORY
  )
