
add_custom_target(check ALL COMMAND check
  $<STRING:ASCII,foo>
  $<STRING:ASCII,256>
  $<STRING:ASCII,32,256>
  $<STRING:ASCII,32,-1>
VERBATIM)
