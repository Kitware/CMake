file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/unknown_dependencies/unknown_dependencies-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/UnknownDependencies-install-check.cmake)
