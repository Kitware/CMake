include(${CMAKE_CURRENT_SOURCE_DIR}/${try_compile_DEFS})

enable_language(C)

set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)

try_compile(result ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  OUTPUT_VARIABLE out
  COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/copy
  COPY_FILE_ERROR copy_err
  )

if(NOT result)
  message(FATAL_ERROR "try_compile failed:\n${out}")
endif()

if(copy_err)
  message(FATAL_ERROR "try_compile COPY_FILE failed:\n${copy_err}")
endif()
