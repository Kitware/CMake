# Test JOB_SERVER_AWARE with custom commands
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/missing"
  JOB_SERVER_AWARE ON
  COMMAND $(CMAKE_COMMAND) -E true
)
add_custom_target(dummy ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/missing")

# Test JOB_SERVER_AWARE with custom targets
add_custom_target(
  dummy2 ALL
  JOB_SERVER_AWARE ON
  COMMAND $(CMAKE_COMMAND) -E true
)

# Test JOB_SERVER_AWARE with custom commands with WORKING_DIRECTORY
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/missing2"
  JOB_SERVER_AWARE ON
  COMMAND $(CMAKE_COMMAND) -E true
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Foo"
)
add_custom_target(dummy3 ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/missing2")

# Test JOB_SERVER_AWARE with custom targets with WORKING_DIRECTORY
add_custom_target(
  dummy4 ALL
  JOB_SERVER_AWARE ON
  COMMAND $(CMAKE_COMMAND) -E true
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Foo"
)
