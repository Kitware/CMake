file(READ "${RunCMake_TEST_BINARY_DIR}/bar.spdx.json" BAR_CONTENT)
file(READ "${RunCMake_TEST_BINARY_DIR}/foo.spdx.json" FOO_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Requirements-install-check.cmake)
