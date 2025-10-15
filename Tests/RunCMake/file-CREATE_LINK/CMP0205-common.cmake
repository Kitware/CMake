# Use COPY_ON_ERROR to handle the case where the source and destination
# directory are on different devices and empty.
file(CREATE_LINK
  ${CMAKE_CURRENT_LIST_DIR}/CMP0205 ${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name}
  ${maybe_SYMBOLIC}
  RESULT result
  COPY_ON_ERROR
  )
if(NOT result STREQUAL "0")
  message(SEND_ERROR "COPY_ON_ERROR failed: '${result}'")
endif()

file(GLOB_RECURSE allFilesSrc LIST_DIRECTORIES true RELATIVE "${CMAKE_CURRENT_LIST_DIR}/CMP0205" "${CMAKE_CURRENT_LIST_DIR}/CMP0205/*")
file(GLOB_RECURSE allFilesDst LIST_DIRECTORIES true RELATIVE "${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name}" "${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name}/*")
