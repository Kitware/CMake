file(READ "${RunCMake_TEST_INSTALL_DIR}/licenses-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/DefaultLicense-install-check.cmake)
