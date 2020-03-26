if(NOT FOO)
  message(FATAL_ERROR "FOO is not set")
endif()

if(NOT "${PROJECT_NAME}" STREQUAL "")
  message(FATAL_ERROR "PROJECT_NAME should be empty")
endif()

set(AUTO_INCLUDE TRUE)
