add_custom_target(custom)
target_sources(custom PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
)

get_source_file_property(prop
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  GENERATED)
message(NOTICE "prop: `${prop}`")
