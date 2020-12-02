add_executable(executable)
target_sources(executable PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.cpp"
)

set_property(SOURCE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.cpp"
  PROPERTY GENERATED "1")
get_property(prop SOURCE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.cpp"
  PROPERTY GENERATED)
message(NOTICE "prop: `${prop}`")
