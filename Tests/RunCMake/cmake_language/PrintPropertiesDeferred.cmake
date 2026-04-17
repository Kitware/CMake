enable_language(C)

add_library(mylib STATIC nothing.c)
set_target_properties(mylib PROPERTIES MY_PROP "mylib_value")

cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  DEFERRED
  NAMED MY_PROP
)
