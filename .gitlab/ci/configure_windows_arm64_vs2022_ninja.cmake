# Qt host tools are not yet available natively on windows-arm64.
set(CMake_TEST_GUI "OFF" CACHE BOOL "")
set(CMake_TEST_TLS_VERIFY_URL "https://gitlab.kitware.com" CACHE STRING "")
set(BUILD_QtDialog "OFF" CACHE BOOL "")
set(CMAKE_PREFIX_PATH "" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_msvc_cxx_modules_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_vs_common_ninja.cmake")
