
if (NOT EXPECTED_LOCATION STREQUAL "64bit")
  message (SEND_ERROR "RegistryViewConfig: location is '64bit' but expects '${EXPECTED_LOCATION}'")
endif()
