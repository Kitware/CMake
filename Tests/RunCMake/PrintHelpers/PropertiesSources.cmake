set_property(SOURCE rot13.c PROPERTY LANGUAGE C)

add_library(rot13 SHARED rot13.c rot13.h)

include(CMakePrintHelpers)

cmake_print_properties(
  TARGETS rot13
  PROPERTIES
    SOURCES
    POSITION_INDEPENDENT_CODE
)

cmake_print_properties(
  SOURCES rot13.c
  PROPERTIES
    LOCATION
    LANGUAGE
)
