file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/baz/baz-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/UninstalledStaticLibrary-install-check.cmake)
