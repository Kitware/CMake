add_custom_target(custom)
target_sources(custom PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
)

get_property(prop SOURCE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.txt"
  PROPERTY GENERATED)
message(NOTICE "prop: `${prop}`")
