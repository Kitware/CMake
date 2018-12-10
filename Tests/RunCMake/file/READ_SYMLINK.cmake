execute_process(COMMAND
  ${CMAKE_COMMAND} -E create_symlink "test.txt" "${CMAKE_CURRENT_BINARY_DIR}/rel.sym")
file(READ_SYMLINK "${CMAKE_CURRENT_BINARY_DIR}/rel.sym" result)
if(NOT result STREQUAL "test.txt")
  message(SEND_ERROR "Relative symlink is \"${result}\", should be \"test.txt\"")
endif()

execute_process(COMMAND
  ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_BINARY_DIR}/test.txt" "${CMAKE_CURRENT_BINARY_DIR}/abs.sym")
file(READ_SYMLINK "${CMAKE_CURRENT_BINARY_DIR}/abs.sym" result)
if(NOT result MATCHES "^.*/Tests/RunCMake/file/READ_SYMLINK-build/test\\.txt$")
  message(SEND_ERROR "Absolute symlink is \"${result}\", should be \"*/Tests/RunCMake/file/READ_SYMLINK-build/test.txt\"")
endif()
