enable_language(C)
add_library(foo empty.c)

cmake_policy(SET CMP0070 NEW)
file(GENERATE OUTPUT relative-output-NEW.c CONTENT "")
target_sources(foo PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/relative-output-NEW.c"
)
