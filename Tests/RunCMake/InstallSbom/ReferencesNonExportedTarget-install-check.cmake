file(READ "${RunCMake_TEST_INSTALL_DIR}/dog.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/ReferencesNonExportedTarget-install-check.cmake)
