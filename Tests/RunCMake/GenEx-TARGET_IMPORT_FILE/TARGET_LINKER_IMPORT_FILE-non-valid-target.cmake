
enable_language(C)

add_executable(exe1 empty.c)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test.txt"
  CONTENT "[$<TARGET_LINKER_IMPORT_FILE:exe1>]"
)
