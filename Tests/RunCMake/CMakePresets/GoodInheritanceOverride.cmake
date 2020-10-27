include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

test_variable(CMAKE_BINARY_DIR "" "${CMAKE_SOURCE_DIR}/build")
test_variable(PARENT_VARIABLE "STRING" "Parent variable")
test_variable(OVERRIDDEN_VARIABLE "STRING" "Overridden variable")
test_variable(CHILD_VARIABLE "STRING" "Child variable")

if(DEFINED DELETED_VARIABLE OR DEFINED CACHE{DELETED_VARIABLE})
  message(SEND_ERROR "DELETED_VARIABLE should not be defined")
endif()

test_environment_variable(PARENT_ENV "Parent environment variable")
test_environment_variable(CHILD_ENV "Child environment variable")
test_environment_variable(OVERRIDDEN_ENV "Overridden environment variable")

if(DEFINED ENV{DELETED_ENV})
  message(SEND_ERROR "DELETED_ENV should not be defined")
endif()
