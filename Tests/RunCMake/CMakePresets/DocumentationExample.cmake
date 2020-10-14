include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

test_variable(FIRST_CACHE_VARIABLE "BOOL" "OFF")
test_variable(SECOND_CACHE_VARIABLE "UNINITIALIZED" "ON")
test_environment_variable(MY_ENVIRONMENT_VARIABLE "Test")
