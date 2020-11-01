add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../GeneratedProperty-build/a"
  COMMAND b
  )
add_custom_target(CollapseFullPath
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/a"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../GeneratedProperty-build/c"
  COMMAND d
  )
get_source_file_property(GENERATED_A "${CMAKE_CURRENT_BINARY_DIR}/a" GENERATED)
get_source_file_property(GENERATED_C "${CMAKE_CURRENT_BINARY_DIR}/c" GENERATED)
if(NOT GENERATED_A OR NOT GENERATED_C)
  message(FATAL_ERROR "failed")
endif()
