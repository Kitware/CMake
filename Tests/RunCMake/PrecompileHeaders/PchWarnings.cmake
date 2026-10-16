set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(C)

add_library(empty empty.c)
target_precompile_headers(empty PUBLIC
  <stdio.h>
  <string.h>
)

add_library(foo foo.c)
target_precompile_headers(foo PUBLIC
  <stdio.h>
  <string.h>
)
set_target_properties(foo PROPERTIES PCH_WARNINGS ON)
