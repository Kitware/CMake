include(CTest)

add_test(
  NAME MultipleExpandOptions
  COMMAND /bin/true
  COMMAND_EXPAND_LISTS
  COMMAND_EXPAND_LISTS
)
