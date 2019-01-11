file(CREATE_LINK ${CMAKE_CURRENT_LIST_FILE} TestLink.cmake RESULT result)
if(NOT result STREQUAL "0")
  message(SEND_ERROR "Hard link result='${result}'")
endif()
