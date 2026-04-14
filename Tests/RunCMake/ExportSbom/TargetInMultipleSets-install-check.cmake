file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/mySbom/mySbom.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/TargetInMultipleSets-install-check.cmake)
