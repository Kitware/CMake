file(READ "${RunCMake_TEST_INSTALL_DIR}/test_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MissingPackageNamespace-install-check.cmake)
