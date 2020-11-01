enable_language(C)
add_library(SourceProperty)

cmake_policy(SET CMP0070 OLD)
file(GENERATE OUTPUT relative-output-OLD.c CONTENT "")
target_sources(SourceProperty PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/relative-output-OLD.c"
)

cmake_policy(SET CMP0070 NEW)
file(GENERATE OUTPUT relative-output-NEW.c CONTENT "")
target_sources(SourceProperty PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/relative-output-NEW.c"
)
