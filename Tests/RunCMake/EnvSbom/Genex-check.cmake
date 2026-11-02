include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Assertions.cmake)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/sbom/800147c20378e0c5769bafa56e83195d/foo-BarConfig.spdx.json" BarConfig)
  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/sbom/800147c20378e0c5769bafa56e83195d/foo-BazConfig.spdx.json" BazConfig)
else()
  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/sbom/800147c20378e0c5769bafa56e83195d/foo-BarConfig.spdx.json" BarConfig)
endif()

set(interface_dependency install_dependency)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Genex-check.cmake)
