file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/test_project/dog.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ReferencesNonExportedTarget-install-check.cmake)
