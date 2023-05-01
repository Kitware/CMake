if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_CPACK_INNOSETUP "ON" CACHE STRING "")
  set(CMake_TEST_ISPC "ON" CACHE STRING "")
endif()
set(CMake_TEST_TLS_VERIFY_URL "https://gitlab.kitware.com" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_msvc_cxx_modules_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_vs_common_ninja.cmake")
