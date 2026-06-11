file(READ "${RunCMake_TEST_INSTALL_DIR}/bar_sbom-Debug.spdx.json" BAR_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceAmbiguity-install-check.cmake)
