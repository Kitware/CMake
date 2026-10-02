enable_language(C)

add_subdirectory(excluded EXCLUDE_FROM_ALL)
add_library(unit_tests STATIC neverbuild.c)
