file(READ "${RunCMake_TEST_INSTALL_DIR}/bar.spdx.json" BAR_CONTENT)
file(READ "${RunCMake_TEST_INSTALL_DIR}/foo.spdx.json" FOO_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Requirements-install-check.cmake)
