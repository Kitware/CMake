# Use COPY_ON_ERROR to handle the case where the source and destination
# directory are on different devices. Cross-device links are not permitted
# and the following command falls back to copying the file if link fails.
file(CREATE_LINK
  ${CMAKE_CURRENT_LIST_FILE} TestCreateLink.cmake
  RESULT result
  COPY_ON_ERROR
  )
if(NOT result STREQUAL "0")
  message(SEND_ERROR "COPY_ON_ERROR failed: '${result}'")
endif()
