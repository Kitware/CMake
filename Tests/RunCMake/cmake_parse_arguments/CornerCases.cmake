include(${CMAKE_CURRENT_LIST_DIR}/test_utils.cmake)

# example from the documentation
# OPTIONAL is a keyword and therefore terminates the definition of
# the multi-value DEFINITION before even a single value has been added

set(options OPTIONAL FAST)
set(oneValueArgs DESTINATION RENAME)
set(multiValueArgs TARGETS CONFIGURATIONS)
cmake_parse_arguments(MY_INSTALL "${options}" "${oneValueArgs}"
                      "${multiValueArgs}"
                      TARGETS foo DESTINATION OPTIONAL)

TEST(MY_INSTALL_DESTINATION UNDEFINED)
TEST(MY_INSTALL_OPTIONAL TRUE)
