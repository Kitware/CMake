# Leave CMP0118 unset!
cmake_policy(SET CMP0163 OLD)

# Call the associated test.
cmake_path(GET RunCMake_TEST STEM testcase)
string(REGEX REPLACE "OLD" "Common" testcase "${testcase}")
include(${CMAKE_CURRENT_LIST_DIR}/../${testcase}.cmake)
