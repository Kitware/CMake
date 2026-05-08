cmake_diagnostic(GET CMD_DEPRECATED action)
if(NOT "${action}" STREQUAL "SEND_ERROR")
  message(SEND_ERROR "CMD_DEPRECATED is ${action}; should be SEND_ERROR")
endif()
