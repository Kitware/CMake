cmake_diagnostic(GET CMD_UNINITIALIZED action)
if(NOT "${action}" STREQUAL SEND_ERROR)
   message(SEND_ERROR
     "wrong action for diagnostic CMD_UNINITIALIZED"
     " (expected 'SEND_ERROR', actual '${action}')")
endif()
