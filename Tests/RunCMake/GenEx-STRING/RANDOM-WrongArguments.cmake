
add_custom_target(check ALL COMMAND check
  $<STRING:RANDOM,LENGTH:foo>
   $<STRING:RANDOM,RANDOM_SEED:foo>
VERBATIM)
