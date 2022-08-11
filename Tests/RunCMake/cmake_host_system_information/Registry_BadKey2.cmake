cmake_host_system_information(RESULT result QUERY WINDOWS_REGISTRY HKLM-SUBKEY ERROR_VARIABLE error)
if (NOT error STREQUAL "")
  message(FATAL_ERROR "${error}")
endif()
