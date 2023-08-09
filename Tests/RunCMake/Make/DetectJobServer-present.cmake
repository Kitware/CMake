# Verifies that the jobserver is present
add_custom_command(OUTPUT custom_command.txt
  JOB_SERVER_AWARE ON
  COMMENT "Should detect jobserver support"
  COMMAND ${DETECT_JOBSERVER} "custom_command.txt"
)

# trigger the custom command to run
add_custom_target(dummy ALL
  JOB_SERVER_AWARE ON
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/custom_command.txt
  COMMAND ${DETECT_JOBSERVER} "custom_target.txt"
)
