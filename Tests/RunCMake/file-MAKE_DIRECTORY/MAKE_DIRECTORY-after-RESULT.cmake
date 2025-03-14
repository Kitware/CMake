file(MAKE_DIRECTORY
  "${CMAKE_CURRENT_BINARY_DIR}/before"
  RESULT resultVal
  "${CMAKE_CURRENT_BINARY_DIR}/after"
)
message(STATUS "Result=${resultVal}")
