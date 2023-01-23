add_custom_target(check ALL COMMAND check
  $<CONFIG:.>
  $<CONFIG:Foo-Bar>
  $<$<CONFIG:Foo-Nested>:foo>
  $<$<CONFIG:Release,Foo-Second>:foo>
  VERBATIM)
