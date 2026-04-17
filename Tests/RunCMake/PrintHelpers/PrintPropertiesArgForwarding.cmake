include(CMakePrintHelpers)
enable_language(C)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/stub.c" "void stub(void) {}\n")

add_library(mydep STATIC "${CMAKE_CURRENT_BINARY_DIR}/stub.c")
set_target_properties(mydep PROPERTIES
  MY_PROP_X "alpha"
  MY_PROP_Y "beta"
  OTHER_PROP "alpha"
)

add_library(mylib STATIC "${CMAKE_CURRENT_BINARY_DIR}/stub.c")
target_link_libraries(mylib PRIVATE mydep)
set_target_properties(mylib PROPERTIES
  MY_PROP_X "alpha"
  MY_PROP_Y "beta"
  OTHER_PROP "alpha"
)

# (1) configure-time, specific properties — exercises forwarding of TARGETS
# + PROPERTIES <names>.
cmake_print_properties(TARGETS mylib PROPERTIES TYPE NAME)

# (2) configure-time, ALL + both regex keywords — exercises forwarding of
# PROPERTY_NAME_REGEX and PROPERTY_VALUE_REGEX together.
cmake_print_properties(
  TARGETS mylib
  PROPERTIES ALL
  PROPERTY_NAME_REGEX "^MY_PROP_"
  PROPERTY_VALUE_REGEX "^alpha$"
)

# (3) DEFERRED + FOLLOW_DEPENDENCIES + ALL + both regexes — exercises
# forwarding of every new keyword at once, and verifies the walker
# actually reaches the PRIVATE dep at generate time (mydep must appear
# in the output, not just mylib).
cmake_print_properties(
  TARGETS mylib
  DEFERRED
  FOLLOW_DEPENDENCIES
  PROPERTIES ALL
  PROPERTY_NAME_REGEX "^MY_PROP_"
  PROPERTY_VALUE_REGEX "^alpha$"
)

# (4) DEFERRED alone + specific name — exercises DEFERRED forwarding by
# itself (no dependency walk; mydep must NOT appear).
cmake_print_properties(TARGETS mylib DEFERRED PROPERTIES TYPE)
