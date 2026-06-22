cmake_diagnostic(SET CMD_EXPERIMENTAL IGNORE)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "01234567-0123-0123-0123-0123456789ab")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)

if (NOT feature_present STREQUAL "FALSE")
  message(FATAL_ERROR
    "Expected the `CxxImportStd` feature to be disabled.")
endif ()

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `CxxImportStd` feature to be enabled.")
endif ()
