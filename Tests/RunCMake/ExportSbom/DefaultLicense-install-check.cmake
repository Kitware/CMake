file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/licenses/licenses-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/DefaultLicense-install-check.cmake)
