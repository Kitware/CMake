cmake_policy(SET CMP0118 NEW)
cmake_policy(SET CMP0163 NEW)

# Call the associated test.
cmake_path(GET RunCMake_TEST STEM testcase)
string(REGEX REPLACE "NEW" "Common" testcase "${testcase}")
include(${CMAKE_CURRENT_LIST_DIR}/../${testcase}.cmake)
