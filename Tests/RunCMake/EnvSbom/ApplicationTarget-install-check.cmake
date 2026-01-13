file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test_project/application_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ApplicationTarget-install-check.cmake)
