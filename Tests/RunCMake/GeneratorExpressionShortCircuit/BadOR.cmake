set(error $<0>)
add_custom_target(check ALL COMMAND check
  $<OR:0,${error}>
)
