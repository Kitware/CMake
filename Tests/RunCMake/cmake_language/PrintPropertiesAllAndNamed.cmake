enable_language(C)
add_library(mylib STATIC nothing.c)

# ALL and NAMED are mutually exclusive.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  ALL
  NAMED MY_PROP
)
