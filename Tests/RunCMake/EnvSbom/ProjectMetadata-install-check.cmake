file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test/test_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ProjectMetadata-install-check.cmake)
