add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/../GeneratedProperty-build/a"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/../GeneratedProperty-build/b"
  COMMAND c
  )
get_source_file_property(GENERATED_A "${CMAKE_CURRENT_BINARY_DIR}/a" GENERATED)
get_source_file_property(GENERATED_B "${CMAKE_CURRENT_BINARY_DIR}/b" GENERATED)
if(NOT GENERATED_A OR NOT GENERATED_B)
  message(FATAL_ERROR "failed")
endif()
