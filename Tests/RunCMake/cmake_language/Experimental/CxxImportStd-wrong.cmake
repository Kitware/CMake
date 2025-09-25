set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "01234567-0123-0123-0123-0123456789ab")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)

if (NOT feature_present STREQUAL "FALSE")
  message(FATAL_ERROR
    "Expected the `CxxImportStd` feature to be disabled.")
endif ()

# Test if/when warning is repeated.
cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "76543210-3210-3210-3210-ba9876543210")
cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)
