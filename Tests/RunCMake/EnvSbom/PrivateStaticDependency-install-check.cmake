file(READ "${RunCMake_TEST_INSTALL_DIR}/lib/sbom/foo/foo-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/PrivateStaticDependency-install-check.cmake)
