add_custom_target(drive
  COMMAND "$<1:$<OUTPUT_CONFIG:a>>"
  COMMAND "$<1:$<COMMAND_CONFIG:b>>"
  )
