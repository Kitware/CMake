
add_custom_target(check ALL COMMAND check
  [[
     $<STRING:REPLACE,FOO,string,.+,>
     $<STRING:REPLACE,REGEX,string,(,>
  ]]
VERBATIM)
