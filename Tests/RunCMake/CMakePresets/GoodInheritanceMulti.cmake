include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

test_variable(CMAKE_BINARY_DIR "" "${CMAKE_SOURCE_DIR}/build")
test_variable(FIRST_VARIABLE "STRING" "First variable")
test_variable(SECOND_VARIABLE "STRING" "Second variable")
test_variable(OVERRIDDEN_VARIABLE "STRING" "Overridden variable")

test_environment_variable(FIRST_ENV "First environment variable")
test_environment_variable(SECOND_ENV "Second environment variable")
test_environment_variable(OVERRIDDEN_ENV "Overridden environment variable")
