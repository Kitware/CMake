enable_language(C)

add_library(mylib STATIC nothing.c)

target_sources(mylib
  PUBLIC
    FILE_SET HEADERS
    FILES nothing.h
)

# Add a property
set_target_properties(mylib
  PROPERTIES
  some_property some_value
  another_property another_value
)

cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  ALL
  PROPERTY_NAME_REGEX INTERFACE
)

cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  ALL
  PROPERTY_VALUE_REGEX HEADERS
)

cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  ALL
  PROPERTY_NAME_REGEX INTERFACE
  PROPERTY_VALUE_REGEX HEADERS
)
