
add_custom_target(check ALL COMMAND check
  $<STRING:FIND,string,OTHER,foo>
  $<STRING:FIND,string,FROM:OTHER,foo>
VERBATIM)
