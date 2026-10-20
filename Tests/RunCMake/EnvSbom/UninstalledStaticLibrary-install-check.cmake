file(READ "${RunCMake_TEST_INSTALL_DIR}/baz-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/UninstalledStaticLibrary-install-check.cmake)
