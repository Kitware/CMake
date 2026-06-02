file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/bar/bar.spdx.json" BAR_CONTENT)
file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo.spdx.json" FOO_CONTENT)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/SbomNamespaceFallback-install-check.cmake)
