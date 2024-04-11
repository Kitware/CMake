set(CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_DEPENDENCIES
  "1942b4fa-b2c5-4546-9385-83f254070067")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "ExportPackageDependencies"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `ExportPackageDependencies` feature to be enabled.")
endif ()
