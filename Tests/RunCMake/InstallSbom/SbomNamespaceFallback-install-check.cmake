file(READ "${RunCMake_TEST_INSTALL_DIR}/bar-Debug.spdx.json" BAR_CONTENT)
file(READ "${RunCMake_TEST_INSTALL_DIR}/foo-Debug.spdx.json" FOO_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceFallback-install-check.cmake)
