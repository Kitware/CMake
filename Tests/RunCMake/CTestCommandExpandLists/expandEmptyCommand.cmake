include(CTest)

set(argv /bin/true)
list(POP_BACK argv)

add_test(
  NAME CommandExpandEmptyList
  COMMAND "$<JOIN:${argv},;>"
  COMMAND_EXPAND_LISTS
)
