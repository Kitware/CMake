enable_testing()

add_test(NAME t COMMAND ${CMAKE_COMMAND} -E echo "Top directory")
add_subdirectory(TEST-subdir1)
add_subdirectory(TEST-subdir2)

set_property(TEST t PROPERTY PASS_REGULAR_EXPRESSION "Top directory")
set_property(TEST t DIRECTORY TEST-subdir1 PROPERTY PASS_REGULAR_EXPRESSION "Subdirectory")
set_property(TEST t2 DIRECTORY "${CMAKE_BINARY_DIR}/TEST-subdir1" PROPERTY PASS_REGULAR_EXPRESSION "Subdirectory")
