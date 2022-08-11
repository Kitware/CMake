
if (NOT EXPECTED_LOCATION STREQUAL "default.32bit")
  message (SEND_ERROR "RegistryViewConfig: location is 'default.32bit' but expects '${EXPECTED_LOCATION}'")
endif()
