
add_custom_target(check ALL COMMAND check
  [[
     $<STRING:QUOTE,FOO,string>
  ]]
VERBATIM)
