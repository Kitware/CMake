add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/custom-command"
  JOB_SERVER_AWARE ON
  COMMAND $(CMAKE_COMMAND) -E touch "${CMAKE_CURRENT_BINARY_DIR}/custom-command"
)
add_custom_target(dummy ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/custom-command")

add_custom_target(
  dummy2 ALL
  JOB_SERVER_AWARE ON
  COMMAND ${CMAKE_COMMAND} -E true
)
