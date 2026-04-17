enable_language(C)

add_library(leaflib STATIC nothing.c)
set_target_properties(leaflib PROPERTIES
  MY_PROP "alpha-leaf"
  MY_OTHER "beta-leaf"
  OTHER_PROP "alpha-other"
)

add_library(mylib STATIC nothing.c)
target_link_libraries(mylib PUBLIC leaflib)
set_target_properties(mylib PROPERTIES
  MY_PROP "alpha-mylib"
  MY_OTHER "beta-mylib"
  OTHER_PROP "alpha-other"
)

# Only properties whose name starts with MY_ AND whose value contains
# "alpha" should print: MY_PROP on both targets.
# MY_OTHER fails the value regex; OTHER_PROP fails the name regex.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  DEFERRED
  FOLLOW_DEPENDENCIES
  ALL
  PROPERTY_NAME_REGEX "^MY_"
  PROPERTY_VALUE_REGEX "alpha"
)
