
add_custom_target(empty)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test.txt"
  CONTENT "[$<TARGET_FILE_SUFFIX:empty>]"
)
