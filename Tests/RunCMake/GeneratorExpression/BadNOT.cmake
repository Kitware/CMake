add_custom_target(check ALL COMMAND check
  $<NOT:>
  $<NOT:,>
  $<NOT:0,1>
  VERBATIM)
