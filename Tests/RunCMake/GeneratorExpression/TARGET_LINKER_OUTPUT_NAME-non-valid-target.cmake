
enable_language(C)

add_custom_target(empty)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test.txt"
  CONTENT "[$<TARGET_LINKER_OUTPUT_NAME:empty>]"
)
