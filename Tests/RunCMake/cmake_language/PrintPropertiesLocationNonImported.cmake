enable_language(C)
add_library(mylib STATIC nothing.c)

# Reading a computed location property from a non-imported target
# is skipped with a warning instead of read. Any other requested properties
# should still be printed.
cmake_language(PRINT_PROPERTIES TARGETS mylib
  NAMED TYPE LOCATION LOCATION_Debug Debug_LOCATION NAME)
