if(WORKFLOW_PRESET STREQUAL "RepeatedPresetArg")
    message(FATAL_ERROR "First preset argument used instead of repeated preset argument")
endif()
