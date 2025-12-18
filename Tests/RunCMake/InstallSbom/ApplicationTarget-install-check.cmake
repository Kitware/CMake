file(READ "${RunCMake_TEST_INSTALL_DIR}/application_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ApplicationTarget-install-check.cmake)
