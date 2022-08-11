
if (NOT EXPECTED_LOCATION STREQUAL "32bit")
  message (SEND_ERROR "RegistryViewConfig: location is '32bit' but expects '${EXPECTED_LOCATION}'")
endif()
