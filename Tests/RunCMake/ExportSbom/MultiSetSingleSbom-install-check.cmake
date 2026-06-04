file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/mySbom/mySbom-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiSetSingleSbom-install-check.cmake)
