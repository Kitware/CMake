
add_custom_target(check ALL COMMAND check
  [[
     $<STRING:MATCH,string,(>
     $<STRING:MATCH,string,FOO,.+>
     $<STRING:MATCH,string,SEEK:FOO,.+>
  ]]
VERBATIM)
