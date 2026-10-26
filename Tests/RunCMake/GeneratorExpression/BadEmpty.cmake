add_custom_target(check ALL COMMAND check
  $<>
  $<:foo>
  VERBATIM)
