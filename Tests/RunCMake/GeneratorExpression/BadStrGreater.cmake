add_custom_target(check ALL COMMAND check
  $<STRGREATER>
  $<STRGREATER:>
  $<STRGREATER:,,>
  $<STRGREATER:something,,>
  VERBATIM)
