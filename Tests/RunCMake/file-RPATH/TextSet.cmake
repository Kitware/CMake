set(f "${CMAKE_CURRENT_BINARY_DIR}/not_a_binary.txt")
file(WRITE "${f}" "Not a binary.\n")
file(RPATH_SET FILE "${f}" NEW_RPATH "/new/rpath")
