enable_language(C)

add_library(mylib STATIC nothing.c)
set_target_properties(mylib PROPERTIES MY_MARKER "marker_value")

# No ALL or NAMED keyword - ALL should be implied.  Narrow with a regex so
# the output is deterministic across cmake builds.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  PROPERTY_NAME_REGEX "^MY_MARKER$"
)
