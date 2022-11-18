add_custom_target(helper)
set_property(TARGET helper PROPERTY MY_TEXT "lorem ipsum")
add_custom_command(
  OUTPUT out.txt
  COMMAND ${CMAKE_COMMAND} -E echo true
  COMMENT "$<TARGET_PROPERTY:helper,MY_TEXT>$<COMMA> $<STREQUAL:foo,bar>$<EQUAL:42,42>"
)
set_property(SOURCE out.txt PROPERTY SYMBOLIC 1)
add_custom_target(main ALL DEPENDS out.txt)
