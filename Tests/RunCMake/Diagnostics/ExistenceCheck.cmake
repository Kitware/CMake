if(NOT DIAGNOSTIC CMD_AUTHOR)
  message(SEND_ERROR "existence check for CMD_AUTHOR failed?!")
endif()

if(DIAGNOSTIC BOGUS)
  message(SEND_ERROR "existence check for bogus category succeeded?!")
endif()
