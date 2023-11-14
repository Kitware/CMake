set(error $<0>)
add_custom_target(check ALL COMMAND check
  $<AND:0,${error}>
  $<AND:0,1,${error}>
  $<AND:1,0,${error}>
  $<AND:0,0,${error}>
)
