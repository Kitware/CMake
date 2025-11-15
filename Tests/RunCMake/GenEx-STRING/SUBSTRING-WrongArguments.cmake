
add_custom_target(check ALL COMMAND check
  $<STRING:SUBSTRING,string,foo,1>
  $<STRING:SUBSTRING,string,1,foo>
  $<STRING:SUBSTRING,string,-1,2>
  $<STRING:SUBSTRING,string,1,-2>
VERBATIM)
