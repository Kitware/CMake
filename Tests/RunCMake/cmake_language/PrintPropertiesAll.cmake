enable_language(C)

add_library(mylib STATIC nothing.c)

target_sources(mylib
  PUBLIC
    FILE_SET HEADERS
    FILES nothing.h
)

add_executable(something something.c something.h)

target_link_libraries(something PUBLIC nothing)

# Printing all for library. To reduce maintenance burden, we only pin a few
# properties in the expected output instead of listing every property reported
# by ALL.
cmake_language(
  PRINT_PROPERTIES
  TARGETS mylib
  ALL
)
