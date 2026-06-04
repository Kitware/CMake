include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Assertions.cmake)

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/sbom/800147c20378e0c5769bafa56e83195d/foo-Debug.spdx.json" DebugSbom)
  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/sbom/800147c20378e0c5769bafa56e83195d/foo-Release.spdx.json" ReleaseSbom)
else()
  file(READ "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/sbom/800147c20378e0c5769bafa56e83195d/foo-Debug.spdx.json" DebugSbom)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Config-check.cmake)
