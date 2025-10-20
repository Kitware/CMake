
add_custom_target(check ALL COMMAND check
  [[
     $<STRING:STRIP,FOO,string>
  ]]
VERBATIM)
