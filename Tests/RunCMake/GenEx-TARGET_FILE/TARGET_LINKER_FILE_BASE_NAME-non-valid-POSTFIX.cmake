
enable_language (C)


add_library (shared1 SHARED empty.c)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test.txt"
  CONTENT "[$<TARGET_LINKER_FILE_BASE_NAME:shared1,FOO>]"
)
