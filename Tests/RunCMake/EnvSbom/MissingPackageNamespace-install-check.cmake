file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test_targets/test_targets-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MissingPackageNamespace-install-check.cmake)
