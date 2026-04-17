enable_language(C)
add_library(mylib STATIC nothing.c)


cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  ALL
  PROPERTY_VALUE_REGEX "[unbalanced"
)
