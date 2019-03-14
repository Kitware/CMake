set(file "${CMAKE_CURRENT_BINARY_DIR}/does-not-exist")

file(SIZE "${file}" CALCULATED_SIZE)
