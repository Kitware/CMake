file(READ "${RunCMake_TEST_BINARY_DIR}/sbom/foo/foo-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/PrivateStaticDependency-install-check.cmake)
