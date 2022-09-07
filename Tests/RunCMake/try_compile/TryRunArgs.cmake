include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

enable_language(C)

try_compile(RESULT ${try_compile_bindir_or_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  COPY_FILE "${CMAKE_CURRENT_BINARY_DIR}/out.bin"
  COMPILE_OUTPUT_VARIABLE compOutputVar
  RUN_OUTPUT_VARIABLE runOutputVar
  RUN_OUTPUT_STDOUT_VARIABLE runOutputStdOutVar
  RUN_OUTPUT_STDERR_VARIABLE runOutputStdErrVar
  WORKING_DIRECTORY runWorkDir
  ARGS runArgs
  )
