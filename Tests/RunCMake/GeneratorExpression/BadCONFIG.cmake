add_custom_target(check ALL COMMAND check
  $<CONFIG:.>
  VERBATIM)
