file(READ "${RunCMake_TEST_INSTALL_DIR}/test_targets-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata-install-check.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadataExplicitAssertions.cmake)
