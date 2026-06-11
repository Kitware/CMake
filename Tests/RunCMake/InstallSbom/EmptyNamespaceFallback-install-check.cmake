file(READ "${RunCMake_TEST_INSTALL_DIR}/mySbom-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/EmptyNamespaceFallback-install-check.cmake)
