enable_language(C)

add_library(leaflib STATIC nothing.c)
set_target_properties(leaflib PROPERTIES
  MY_PROP "leaf"
  MY_PROP_2 "leaf2"
)

add_library(linkonlylib STATIC nothing.c)
set_target_properties(linkonlylib PROPERTIES
  MY_PROP "linkonly"
  MY_PROP_2 "linkonly2"
)

add_library(intermediate STATIC nothing.c)
target_link_libraries(intermediate
  PRIVATE leaflib
  INTERFACE $<LINK_ONLY:linkonlylib>
)
set_target_properties(intermediate PROPERTIES
  MY_PROP "intermediate"
  MY_PROP_2 "intermediate2"
)

add_library(mylib STATIC nothing.c)
target_link_libraries(mylib PUBLIC intermediate)
set_target_properties(mylib PROPERTIES
  MY_PROP "mylib"
  MY_PROP_2 "mylib2"
)

# ALL + FOLLOW_DEPENDENCIES with a tight name regex pins the unified
# printer's ALL branch: each reachable target's matching properties show
# up in walk order, covering PRIVATE and $<LINK_ONLY:> traversal.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  DEFERRED
  FOLLOW_DEPENDENCIES
  ALL
  PROPERTY_NAME_REGEX "^MY_PROP"
)
