file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/shared_targets/shared_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SharedTarget-install-check.cmake)
