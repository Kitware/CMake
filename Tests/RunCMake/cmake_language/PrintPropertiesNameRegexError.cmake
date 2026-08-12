enable_language(C)
add_library(mylib STATIC nothing.c)

# Unbalanced bracket - regex compile must fail with a fatal error.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  ALL
  PROPERTY_NAME_REGEX "[unbalanced"
)
