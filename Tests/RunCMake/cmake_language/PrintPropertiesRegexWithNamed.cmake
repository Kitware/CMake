enable_language(C)
add_library(mylib STATIC nothing.c)

# Regex filters require ALL; combining with NAMED is an error.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  NAMED MY_PROP
  PROPERTY_NAME_REGEX "MY_"
)
