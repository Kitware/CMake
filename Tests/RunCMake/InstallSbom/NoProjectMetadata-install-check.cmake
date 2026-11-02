file(READ "${RunCMake_TEST_INSTALL_DIR}/metadata-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/NoProjectMetadata-install-check.cmake)
