file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/dog/dog.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ReferencesNonExportedTarget-install-check.cmake)
