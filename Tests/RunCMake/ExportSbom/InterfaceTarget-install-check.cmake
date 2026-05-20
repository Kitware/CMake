file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/interface_targets/interface_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/InterfaceTarget-install-check.cmake)
