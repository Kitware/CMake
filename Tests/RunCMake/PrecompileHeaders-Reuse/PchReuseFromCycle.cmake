enable_language(C)

add_library(first foo.c)
target_precompile_headers(first REUSE_FROM third)

add_library(second foo.c)
target_precompile_headers(second REUSE_FROM first)

add_library(third foo.c)
target_precompile_headers(third REUSE_FROM second)
