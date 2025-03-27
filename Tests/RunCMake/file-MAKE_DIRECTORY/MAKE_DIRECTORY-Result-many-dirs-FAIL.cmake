file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/file" "")

file(MAKE_DIRECTORY
  "${CMAKE_CURRENT_BINARY_DIR}/file/directory0"
  "${CMAKE_CURRENT_BINARY_DIR}/file/directory1"
  "${CMAKE_CURRENT_BINARY_DIR}/file/directory2"
  RESULT resultVal
)
message(STATUS "Result=${resultVal}")
