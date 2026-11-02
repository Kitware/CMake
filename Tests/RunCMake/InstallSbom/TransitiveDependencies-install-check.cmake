file(READ "${RunCMake_TEST_INSTALL_DIR}/traversal-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/TransitiveDependencies-install-check.cmake)
