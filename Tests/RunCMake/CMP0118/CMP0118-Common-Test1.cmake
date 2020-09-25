add_executable(executable)
target_sources(executable PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.cpp"
)

get_property(prop SOURCE
  "${CMAKE_CURRENT_BINARY_DIR}/GeneratedMain.cpp"
  PROPERTY GENERATED)
message(NOTICE "prop: `${prop}`")
