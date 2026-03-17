cmake_diagnostic(GET CMD_UNUSED_CLI action)
if(NOT "${action}" STREQUAL SEND_ERROR)
   message(SEND_ERROR
     "wrong action for diagnostic CMD_UNUSED_CLI"
     " (expected 'SEND_ERROR', actual '${action}')")
endif()
