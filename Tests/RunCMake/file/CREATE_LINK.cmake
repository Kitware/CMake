# start with a file in the same directory to avoid cross-device links
set(test_file ${CMAKE_CURRENT_BINARY_DIR}/CreateLinkTest.txt)
file(TOUCH ${test_file})

file(CREATE_LINK
  ${test_file} ${CMAKE_CURRENT_BINARY_DIR}/TestCreateLink.txt
  RESULT result
  )
if(NOT result STREQUAL "0")
  message(SEND_ERROR "Hard link result='${result}'")
endif()
