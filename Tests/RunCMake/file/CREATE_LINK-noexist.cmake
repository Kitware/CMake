file(CREATE_LINK does_not_exist.txt TestLink.txt RESULT result)
if(NOT result STREQUAL "0")
  message("Hard link error: ${result}")
endif()
