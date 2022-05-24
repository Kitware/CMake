enable_language(C)

set(ENV{CMAKE_BUILD_TYPE} "Bad")
set(ENV{CMAKE_CONFIGURATION_TYPES} "Bad;Debug")

add_library(tc_defs INTERFACE IMPORTED)
target_compile_definitions(tc_defs INTERFACE "TC_CONFIG_$<UPPER_CASE:$<CONFIG>>")

try_compile(ENV_CONFIG_RESULT "${CMAKE_BINARY_DIR}"
  SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/EnvConfig.c"
  COPY_FILE "${CMAKE_CURRENT_BINARY_DIR}/EnvConfig.bin"
  OUTPUT_VARIABLE tc_output
  LINK_LIBRARIES tc_defs
  )
if(NOT ENV_CONFIG_RESULT)
  string(REPLACE "\n" "\n  " tc_output "  ${tc_output}")
  message(FATAL_ERROR "try_compile failed:\n${tc_output}")
endif()
