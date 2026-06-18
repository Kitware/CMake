set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "CxxImportStd"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `CxxImportStd` feature to be enabled.")
endif ()
