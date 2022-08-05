
add_custom_target(check ALL COMMAND check
  $<PATH_EQUAL>
  $<PATH_EQUAL:>
  $<PATH_EQUAL:,,>
  $<PATH_EQUAL:something,,>
  VERBATIM)
