if(NOT WORKFLOW_PRESET STREQUAL "SinglePresetArg")
    message(FATAL_ERROR "Expected 'SinglePresetArg' workflow preset, but actual value is ${WORKFLOW_PRESET}")
endif()
