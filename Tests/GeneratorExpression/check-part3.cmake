
include(${CMAKE_CURRENT_LIST_DIR}/check-common.cmake)

check(test_version_greater_1 "0")
check(test_version_greater_2 "1")
check(test_version_less_1 "0")
check(test_version_less_2 "1")
check(test_version_equal_1 "0")
check(test_version_equal_2 "1")
