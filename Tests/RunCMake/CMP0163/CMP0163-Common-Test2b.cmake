add_custom_target(custom)
target_sources(custom PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
)

set_source_files_properties(
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  PROPERTIES GENERATED "1")
get_source_file_property(prop
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  GENERATED)
message(NOTICE "prop: `${prop}`")
