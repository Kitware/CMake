file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/interface_targets/interface_targets-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/InterfaceTarget-install-check.cmake)
