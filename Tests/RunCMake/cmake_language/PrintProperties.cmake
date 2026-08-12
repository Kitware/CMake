enable_language(C)

set_property(SOURCE nothing.c PROPERTY LANGUAGE C)
set_property(SOURCE something.c PROPERTY
  COMPILE_DEFINITIONS SOMETHING=1)

add_library(nothing STATIC nothing.c nothing.h)

add_executable(something something.c something.h)
target_link_libraries(something PUBLIC nothing)

cmake_language(
  PRINT_PROPERTIES
  TARGETS nothing something
  NAMED
    LINKER_LANGUAGE
    TYPE
)

cmake_language(
  PRINT_PROPERTIES
  SOURCES nothing.c something.c
  NAMED
    COMPILE_DEFINITIONS
    LANGUAGE
)
