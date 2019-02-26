get_property(role GLOBAL PROPERTY CMAKE_ROLE)

file(WRITE "${CMAKE_BINARY_DIR}/test.cmake" "# a")
include("${CMAKE_BINARY_DIR}/test.cmake")

if(NOT role STREQUAL "PROJECT")
  message(SEND_ERROR "CMAKE_ROLE property is \"${role}\", should be \"PROJECT\"")
endif()

add_subdirectory(sub)
