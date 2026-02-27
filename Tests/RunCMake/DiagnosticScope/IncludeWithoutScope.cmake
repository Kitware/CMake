cmake_diagnostic(SET CMD_AUTHOR WARN)
include(DiagnosticInclude.cmake NO_DIAGNOSTIC_SCOPE)

cmake_diagnostic(GET CMD_AUTHOR action)
if(NOT "${action}" STREQUAL "SEND_ERROR")
  message(SEND_ERROR "include unexpectedly encapsulated diagnostic state")
endif()
