if(NOT "${TEST_PRESET_NAME}" STREQUAL "ConfigurePresetName")
  message(FATAL_ERROR
    "Expected preset name 'ConfigurePresetName', got '${TEST_PRESET_NAME}'")
endif()

if(NOT "${TEST_CONFIGURE_PRESET_NAME}" STREQUAL "${TEST_PRESET_NAME}")
  message(FATAL_ERROR
    "Expected configure preset name '${TEST_PRESET_NAME}', got '${TEST_CONFIGURE_PRESET_NAME}'")
endif()
