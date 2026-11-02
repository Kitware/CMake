file(READ "${RunCMake_TEST_INSTALL_DIR}/foo-Debug.spdx.json" content)
include(${CMAKE_CURRENT_LIST_DIR}/../Sbom/PrivateStaticDependency-install-check.cmake)
