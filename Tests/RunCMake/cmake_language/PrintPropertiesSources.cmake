enable_language(C)

set_property(SOURCE rot13.c PROPERTY LANGUAGE C)

add_library(rot13 SHARED rot13.c rot13.h)

cmake_language(
  PRINT_PROPERTIES
  TARGETS rot13
  NAMED
    SOURCES
    POSITION_INDEPENDENT_CODE
)

cmake_language(
  PRINT_PROPERTIES
  SOURCES rot13.c
  NAMED
    LOCATION
    LANGUAGE
)
