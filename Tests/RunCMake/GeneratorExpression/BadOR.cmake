add_custom_target(check ALL COMMAND check
  $<OR:>
  $<OR:,>
  VERBATIM)
