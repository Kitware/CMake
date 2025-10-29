file(READ "${RunCMake_TEST_INSTALL_DIR}/shared_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SharedTarget-install-check.cmake)
