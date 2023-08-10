if("$ENV{CMAKE_CI_BOOTSTRAP}")
  # Launchers do not work during bootstrap: no ctest available.
  set(CTEST_USE_LAUNCHERS "OFF" CACHE BOOL "")
  # We configure by bootstrapping, so skip the BootstrapTest.
  set(CMake_TEST_BOOTSTRAP OFF CACHE BOOL "")
else()
  set(CTEST_USE_LAUNCHERS "ON" CACHE BOOL "")
endif()

# We run the install right after the build. Avoid rerunning it when installing.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY "ON" CACHE BOOL "")
# Install CMake under the build tree.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "")
set(CMake_TEST_INSTALL "OFF" CACHE BOOL "")

if (NOT "$ENV{CMAKE_CI_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "$ENV{CMAKE_CI_BUILD_TYPE}" CACHE STRING "")
endif ()

if (NOT configure_no_sccache)
  include("${CMAKE_CURRENT_LIST_DIR}/configure_sccache.cmake")
endif()
