file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test_project/test_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MissingPackageNamespace-install-check.cmake)
