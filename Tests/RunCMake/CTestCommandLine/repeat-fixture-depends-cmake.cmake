# A test outside a repeat group that DEPENDS on a test inside it has to wait
# for the group's last repetition, not its first.
cmake_policy(SET CMP0224 NEW)
include(${CMAKE_CURRENT_LIST_DIR}/repeat-fixture-common.cmake)

add_test(NAME after_fixture COMMAND ${CMAKE_COMMAND} -E true)
set_tests_properties(after_fixture PROPERTIES DEPENDS test_with_fixture)
