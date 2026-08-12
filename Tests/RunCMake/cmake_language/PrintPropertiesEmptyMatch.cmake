enable_language(C)

add_library(libdep INTERFACE)
add_library(mylib STATIC nothing.c)
target_link_libraries(mylib PRIVATE libdep)

# One call per regex so each warning text is short and won't wrap.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  DEFERRED
  FOLLOW_DEPENDENCIES
  ALL
  PROPERTY_NAME_REGEX "ZZZ_NO_SUCH_PROPERTY"
)

cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  DEFERRED
  FOLLOW_DEPENDENCIES
  ALL
  PROPERTY_VALUE_REGEX "ZZZ_NO_SUCH_VALUE"
)
