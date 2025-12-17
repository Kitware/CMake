enable_language(C)
set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()
set(CMAKE_C_LINK_FLAGS ${pre}BADFLAG${obj})

try_compile(RESULT ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  OUTPUT_VARIABLE out
)

if (RESULT)
  message(STATUS "try_compile output: ${out}")
else()
  message(FATAL_ERROR "try_compile output: ${out}")
endif()
