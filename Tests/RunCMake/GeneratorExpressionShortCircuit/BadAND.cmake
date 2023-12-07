set(error $<0>)
add_custom_target(check ALL COMMAND check
  $<AND:1,${error}>
)
