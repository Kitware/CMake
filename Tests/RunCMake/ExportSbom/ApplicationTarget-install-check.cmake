file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/application_targets/application_targets-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ApplicationTarget-install-check.cmake)
