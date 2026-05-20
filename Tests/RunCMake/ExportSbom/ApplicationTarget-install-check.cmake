file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/application_targets/application_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ApplicationTarget-install-check.cmake)
