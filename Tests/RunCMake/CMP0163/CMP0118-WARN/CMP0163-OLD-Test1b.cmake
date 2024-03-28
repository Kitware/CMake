# Leave CMP0118 unset!
# Extract from the current file the name of the test-case and value for CMP0163 and call the test-case.
cmake_path(GET CMAKE_CURRENT_LIST_FILE STEM testcase)
string(REGEX MATCH "NEW|OLD|WARN" value_for_CMP0163 "${testcase}")
string(REGEX REPLACE "${value_for_CMP0163}" "Common" testcase "${testcase}")
include(${CMAKE_CURRENT_LIST_DIR}/../${testcase}.cmake)
