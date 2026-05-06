cmake_diagnostic(GET CMD_DEPRECATED action)
if(NOT "${action}" STREQUAL "IGNORE")
  message(SEND_ERROR "CMD_DEPRECATED is ${action}; should be IGNORE")
endif()
