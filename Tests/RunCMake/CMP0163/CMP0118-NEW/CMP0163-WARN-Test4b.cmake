cmake_policy(SET CMP0118 NEW)
# Leave CMP0163 unset!

# Call the associated test.
cmake_path(GET RunCMake_TEST STEM testcase)
string(REGEX REPLACE "WARN" "Common" testcase "${testcase}")
include(${CMAKE_CURRENT_LIST_DIR}/../${testcase}.cmake)
