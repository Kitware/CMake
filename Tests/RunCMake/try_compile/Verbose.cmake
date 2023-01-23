enable_language(C)

try_compile(COMPILE_RESULT
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Verbose.c
  COMPILE_DEFINITIONS -DEXAMPLE_DEFINITION
  OUTPUT_VARIABLE out
  )
string(REPLACE "\n" "\n  " out "  ${out}")
if(NOT COMPILE_RESULT)
  message(FATAL_ERROR "try_compile failed:\n${out}")
endif()
if(NOT out MATCHES "EXAMPLE_DEFINITION"
    AND NOT CMAKE_GENERATOR MATCHES "NMake|Borland")
  message(FATAL_ERROR "try_compile output does not contain EXAMPLE_DEFINITION:\n${out}")
endif()
