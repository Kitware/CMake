cmake_diagnostic(SET CMD_AUTHOR WARN)
include(DiagnosticInclude.cmake)

cmake_diagnostic(GET CMD_AUTHOR action)
if(NOT "${action}" STREQUAL "WARN")
  message(SEND_ERROR "include unexpectedly leaked diagnostic state")
endif()
