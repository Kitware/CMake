set(f "${CMAKE_CURRENT_BINARY_DIR}/not_a_binary.txt")
file(WRITE "${f}" "Not a binary.\n")
file(RPATH_REMOVE FILE "${f}")
