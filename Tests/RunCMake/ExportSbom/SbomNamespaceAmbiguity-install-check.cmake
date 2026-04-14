file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/bar_sbom/bar_sbom.spdx.json" BAR_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceAmbiguity-install-check.cmake)
