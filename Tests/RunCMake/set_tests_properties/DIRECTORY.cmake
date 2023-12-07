enable_testing()

add_test(NAME t COMMAND ${CMAKE_COMMAND} -E echo "Top directory")
add_subdirectory(DIRECTORY-subdir1)
add_subdirectory(DIRECTORY-subdir2)

set_tests_properties(t PROPERTIES PASS_REGULAR_EXPRESSION "Top directory")
set_tests_properties(t DIRECTORY DIRECTORY-subdir1 PROPERTIES PASS_REGULAR_EXPRESSION "Subdirectory")
set_tests_properties(t2 DIRECTORY "${CMAKE_BINARY_DIR}/DIRECTORY-subdir1" PROPERTIES PASS_REGULAR_EXPRESSION "Subdirectory")
