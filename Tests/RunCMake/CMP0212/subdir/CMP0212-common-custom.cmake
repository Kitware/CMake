add_custom_command(
  OUTPUT cmd1
  COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/cmd1"
  DEPENDS foo.exe
)
add_custom_target(tgt1 DEPENDS cmd1)
