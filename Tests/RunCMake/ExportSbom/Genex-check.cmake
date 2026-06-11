include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Assertions.cmake)

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo-BarConfig.spdx.json" BarConfig)
  file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo-BazConfig.spdx.json" BazConfig)
else()
  file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo-BarConfig.spdx.json" BarConfig)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Genex-check.cmake)
