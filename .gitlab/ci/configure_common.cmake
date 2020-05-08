set(CTEST_USE_LAUNCHERS "ON" CACHE STRING "")

# We run the install right after the build. Avoid rerunning it when installing.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY "ON" CACHE STRING "")
# Install CMake under the build tree.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "")
set(CMake_TEST_INSTALL "OFF" CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_sccache.cmake")
