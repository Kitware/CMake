add_custom_target(check ALL COMMAND check
  $<STRLESS>
  $<STRLESS:>
  $<STRLESS:,,>
  $<STRLESS:something,,>
  VERBATIM)
