file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
  "Some detailed output information!\n")
file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
  "Some detailed error information!\n")
message(SEND_ERROR "Some error!")
