add_custom_target(custom)
target_sources(custom PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
)

set_property(SOURCE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  PROPERTY GENERATED "1")
get_source_file_property(prop
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  GENERATED)
message(NOTICE "prop: `${prop}`")
