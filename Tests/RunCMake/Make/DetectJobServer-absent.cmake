# Verifies that the jobserver connection is absent
add_custom_command(OUTPUT custom_command.txt
  JOB_SERVER_AWARE OFF
  COMMENT "Should not detect jobserver"
  COMMAND ${DETECT_JOBSERVER} --absent "custom_command.txt"
)

# trigger the custom command to run
add_custom_target(dummy ALL
  JOB_SERVER_AWARE OFF
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/custom_command.txt
  COMMAND ${DETECT_JOBSERVER} --absent "custom_target.txt"
)
