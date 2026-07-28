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
  "25d6f6aa-be65-4692-b44e-87b23e96d4e1")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `CxxImportStd` feature to be enabled.")
endif ()
