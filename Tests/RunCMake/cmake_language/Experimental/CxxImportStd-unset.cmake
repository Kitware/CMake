cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)

if (NOT feature_present STREQUAL "FALSE")
  message(FATAL_ERROR
    "Expected the `CxxImportStd` feature to be disabled.")
endif ()
