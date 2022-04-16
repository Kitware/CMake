
if (NOT EXPECTED_LOCATION STREQUAL "default.64bit")
  message (SEND_ERROR "RegistryViewConfig: location is 'default.64bit' but expects '${EXPECTED_LOCATION}'")
endif()
