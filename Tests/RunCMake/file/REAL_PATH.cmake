
file(TOUCH "${CMAKE_CURRENT_BINARY_DIR}/test.txt")
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/test.sym")
file(CREATE_LINK  "test.txt" "${CMAKE_CURRENT_BINARY_DIR}/test.sym" SYMBOLIC)

file(REAL_PATH "${CMAKE_CURRENT_BINARY_DIR}/test.sym" real_path)
if (NOT real_path STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/test.txt")
  message(SEND_ERROR "real path is \"${real_path}\", should be \"${CMAKE_CURRENT_BINARY_DIR}/test.txt\"")
endif()

file(REAL_PATH "test.sym" real_path BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
if (NOT real_path STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/test.txt")
  message(SEND_ERROR "real path is \"${real_path}\", should be \"${CMAKE_CURRENT_BINARY_DIR}/test.txt\"")
endif()
