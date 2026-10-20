file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/metadata/metadata-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/NoProjectMetadata-install-check.cmake)
