enable_language(C)

add_library(leaflib STATIC nothing.c)
set_target_properties(leaflib PROPERTIES MY_PROP "leaf")

add_library(mylib STATIC nothing.c)
target_link_libraries(mylib PUBLIC leaflib)
set_target_properties(mylib PROPERTIES MY_PROP "mylib")

cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  FOLLOW_DEPENDENCIES
  NAMED MY_PROP
)
