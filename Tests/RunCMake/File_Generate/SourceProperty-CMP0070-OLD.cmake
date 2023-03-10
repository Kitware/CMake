enable_language(C)
add_library(foo empty.c)

cmake_policy(SET CMP0070 OLD)
file(GENERATE OUTPUT relative-output-OLD.c CONTENT "")
target_sources(foo PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/relative-output-OLD.c"
)
