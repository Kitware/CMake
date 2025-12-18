file(READ "${RunCMake_TEST_BINARY_DIR}/interface_targets.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/MissingPackageNamespace-install-check.cmake)
