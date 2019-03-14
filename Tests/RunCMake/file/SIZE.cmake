set(file "${CMAKE_CURRENT_BINARY_DIR}/a-test-file")

file(WRITE "${file}" "test")

file(SIZE "${file}" CALCULATED_SIZE)

if (NOT CALCULATED_SIZE EQUAL 4)
  message(FATAL_ERROR "Unexpected file size")
endif()
