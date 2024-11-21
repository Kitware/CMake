set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "a9e1cf81-9932-4810-974b-6eccaf14e457")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `CxxImportStd` feature to be enabled.")
endif ()
