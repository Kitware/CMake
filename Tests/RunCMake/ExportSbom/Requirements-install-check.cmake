file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/bar_sbom/bar_sbom.spdx.json" BAR_CONTENT)
file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo_sbom/foo_sbom.spdx.json" FOO_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/Requirements-install-check.cmake)
