enable_language(C)

add_subdirectory(subdir)
add_library(unit_tests STATIC neverbuild.c)
