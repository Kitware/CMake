
add_custom_target(check ALL COMMAND check
  $<STRING:HASH,string,ALGORITHM:>
  $<STRING:HASH,string,ALGORITHM:FOO>
VERBATIM)
