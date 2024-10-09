cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "Unknown"
  feature_present)

if (NOT feature_present STREQUAL "")
  message(FATAL_ERROR
    "Got a result for the `Unknown` experimental feature.")
endif ()
