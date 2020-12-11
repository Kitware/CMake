add_custom_target(custom)
target_sources(custom PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
)

set_property(SOURCE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  PROPERTY GENERATED "1")
get_property(prop SOURCE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  PROPERTY GENERATED)
message(NOTICE "prop: `${prop}`")
