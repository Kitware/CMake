add_custom_target(Custom)

# Write a file that GNU make will prefer over "Makefile"
# if 'cmake --build' does not explicitly specify it.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/GNUmakefile" "")
