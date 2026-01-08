set(CMake_TEST_C_STANDARDS "90;99;11;17;23" CACHE STRING "")
set(CMake_TEST_CXX_STANDARDS "98;11;14;17;20;23;26" CACHE STRING "")

set(CMake_TEST_FindOpenAL "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenMP "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenMP_C "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenMP_CXX "ON" CACHE BOOL "")
set(CMake_TEST_GUI "ON" CACHE BOOL "")
if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_ISPC "ON" CACHE STRING "")
endif()
set(CMake_TEST_TLS_VERIFY_URL "https://gitlab.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERIFY_URL_BAD "https://badtls-expired.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERSION "1.2" CACHE STRING "")
set(CMake_TEST_TLS_VERSION_URL_BAD "https://badtls-v1-1.kitware.com:8011" CACHE STRING "")

# "Release" flags without "-DNDEBUG" so we get assertions.
set(CMAKE_C_FLAGS_RELEASE "-O3" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "")

# https://libcxx.llvm.org/Hardening.html
set(CMAKE_CXX_FLAGS "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
