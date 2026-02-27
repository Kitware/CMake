cmake_diagnostic(SET CMD_AUTHOR IGNORE)

cmake_diagnostic(GET CMD_AUTHOR action)
if(NOT "${action}" STREQUAL "IGNORE")
  message(SEND_ERROR "failed to set diagnostic state")
endif()

block(SCOPE_FOR DIAGNOSTICS)

cmake_diagnostic(SET CMD_AUTHOR SEND_ERROR)

cmake_diagnostic(GET CMD_AUTHOR action)
if(NOT "${action}" STREQUAL "SEND_ERROR")
  message(SEND_ERROR "failed to set diagnostic state")
endif()

endblock()

cmake_diagnostic(GET CMD_AUTHOR action)
if(NOT "${action}" STREQUAL "IGNORE")
  message(SEND_ERROR "failed to restore diagnostic state")
endif()
