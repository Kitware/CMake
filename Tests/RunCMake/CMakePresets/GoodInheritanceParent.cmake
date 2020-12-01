include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

test_variable(CMAKE_BINARY_DIR "" "${CMAKE_SOURCE_DIR}/build")
test_variable(TEST_VARIABLE "STRING" "Some string")

test_environment_variable(TEST_ENV "Some environment variable")
