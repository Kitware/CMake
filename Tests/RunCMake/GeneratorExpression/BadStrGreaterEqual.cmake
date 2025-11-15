add_custom_target(check ALL COMMAND check
  $<STRGREATER_EQUAL>
  $<STRGREATER_EQUAL:>
  $<STRGREATER_EQUAL:,,>
  $<STRGREATER_EQUAL:something,,>
  VERBATIM)
