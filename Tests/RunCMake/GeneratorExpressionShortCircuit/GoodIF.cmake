set(error $<0>)
add_custom_target(check ALL
  COMMAND check $<IF:1,1,${error}>
  COMMAND Check $<IF:0,${error},1>
)
