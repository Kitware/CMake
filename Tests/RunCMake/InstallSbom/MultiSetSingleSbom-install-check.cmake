file(READ "${RunCMake_TEST_INSTALL_DIR}/mySbom.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MultiSetSingleSbom-install-check.cmake)
