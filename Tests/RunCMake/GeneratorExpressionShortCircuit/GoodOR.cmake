set(error $<0>)
add_custom_target(check ALL COMMAND check
  $<OR:1,${error}>
  $<OR:0,1,${error}>
  $<OR:1,0,${error}>
)
