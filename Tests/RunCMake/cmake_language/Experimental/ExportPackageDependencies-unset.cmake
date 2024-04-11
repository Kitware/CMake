cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "ExportPackageDependencies"
  feature_present)

if (NOT feature_present STREQUAL "FALSE")
  message(FATAL_ERROR
    "Expected the `ExportPackageDependencies` feature to be disabled.")
endif ()
