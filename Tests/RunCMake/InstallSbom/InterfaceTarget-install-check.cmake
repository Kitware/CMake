file(READ "${RunCMake_TEST_INSTALL_DIR}/interface_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/InterfaceTarget-install-check.cmake)
