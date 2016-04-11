enable_language(C)
set(CMAKE_C_FLAGS_RELEASE "-DPP_ERROR ${CMAKE_C_FLAGS_DEBUG}")

#-----------------------------------------------------------------------------
set(CMAKE_TRY_COMPILE_CONFIGURATION Release)
try_compile(RESULT ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src.c
  OUTPUT_VARIABLE out
  )
string(REPLACE "\n" "\n  " out "  ${out}")
if(RESULT)
  message(FATAL_ERROR "try_compile passed but should have failed:\n${out}")
elseif(NOT "x${out}" MATCHES "PP_ERROR is defined")
  message(FATAL_ERROR "try_compile did not fail with PP_ERROR:\n${out}")
else()
  message(STATUS "try_compile with per-config flag worked as expected")
endif()
