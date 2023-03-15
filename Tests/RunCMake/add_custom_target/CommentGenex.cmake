add_custom_target(helper)
set_property(TARGET helper PROPERTY MY_TEXT "lorem ipsum")
add_custom_target(main ALL
  COMMAND ${CMAKE_COMMAND} -E true
  COMMENT "$<TARGET_PROPERTY:helper,MY_TEXT>$<COMMA> $<STREQUAL:foo,bar>$<EQUAL:42,42>"
)
