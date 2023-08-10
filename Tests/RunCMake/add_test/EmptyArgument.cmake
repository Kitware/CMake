enable_testing()
add_test(
  NAME "EmptyArgument"
  COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/CheckEmptyArgument.cmake" -- "A" "" "B"
)
