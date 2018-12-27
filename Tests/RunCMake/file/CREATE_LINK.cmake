file(CREATE_LINK ${CMAKE_CURRENT_LIST_FILE} TestLink.cmake RESULT result)
if(NOT result STREQUAL "0")
  message(SEND_ERROR "Hard link result='${result}'")
endif()

file(CREATE_LINK ${CMAKE_CURRENT_LIST_FILE} TestSymLink.cmake RESULT sym_result SYMBOLIC)
if(NOT sym_result STREQUAL "0")
  message(SEND_ERROR "Symlink result='${sym_result}'")
endif()
