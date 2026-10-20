file(READ "${RunCMake_TEST_INSTALL_DIR}/bar_sbom-Debug.spdx.json" BAR_CONTENT)
file(READ "${RunCMake_TEST_INSTALL_DIR}/foo_sbom-Debug.spdx.json" FOO_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Requirements-install-check.cmake)
