enable_language(C)

set_property(SOURCE nothing.c PROPERTY LANGUAGE C)
set_property(SOURCE something.c PROPERTY
  COMPILE_DEFINITIONS SOMETHING=1)

add_library(nothing STATIC nothing.c nothing.h)

add_executable(something something.c something.h)
target_link_libraries(something PUBLIC nothing)

include(CMakePrintHelpers)

cmake_print_properties(
  TARGETS nothing something
  PROPERTIES
    LINKER_LANGUAGE
    TYPE
)

cmake_print_properties(
  SOURCES nothing.c something.c
  PROPERTIES
    COMPILE_DEFINITIONS
    LANGUAGE
)
