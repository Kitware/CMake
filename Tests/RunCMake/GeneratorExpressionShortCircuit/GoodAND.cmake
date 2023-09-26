set(error $<0>)
add_custom_target(check ALL COMMAND check
  $<AND:0,${error}>
)
