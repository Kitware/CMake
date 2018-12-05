file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rel.sym" "")
file(READ_SYMLINK "${CMAKE_CURRENT_BINARY_DIR}/rel.sym" result)
