file(CREATE_LINK does_not_exist.txt TestLink.txt RESULT result)
if(NOT result STREQUAL "0")
  message("Hard link error: ${result}")
endif()

file(CREATE_LINK does_not_exist.txt TestSymLink.txt RESULT sym_result SYMBOLIC)
if(NOT sym_result STREQUAL "0")
  message("Symlink fail: ${sym_result}")
endif()
