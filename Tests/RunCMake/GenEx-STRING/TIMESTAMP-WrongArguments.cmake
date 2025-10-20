
add_custom_target(check ALL COMMAND check
  $<STRING:TIMESTAMP,%s,%s>
VERBATIM)
