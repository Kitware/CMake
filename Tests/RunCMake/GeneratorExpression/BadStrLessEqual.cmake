add_custom_target(check ALL COMMAND check
  $<STRLESS_EQUAL>
  $<STRLESS_EQUAL:>
  $<STRLESS_EQUAL:,,>
  $<STRLESS_EQUAL:something,,>
  VERBATIM)
