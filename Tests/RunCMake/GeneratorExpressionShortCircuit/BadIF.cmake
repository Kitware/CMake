set(error $<0>)
add_custom_target(check ALL COMMAND check
  $<IF:0,1,${error}>
)
