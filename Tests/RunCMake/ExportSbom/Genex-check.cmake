include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Assertions.cmake)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo-BarConfig.spdx.json" BarConfig)
  file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo-BazConfig.spdx.json" BazConfig)
else()
  file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo-BarConfig.spdx.json" BarConfig)
endif()

set(interface_dependency build_dependency)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Genex-check.cmake)
