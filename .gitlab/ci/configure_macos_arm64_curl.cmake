# Build with our vendored curl instead of the default system version.
set(CMAKE_USE_SYSTEM_CURL "OFF" CACHE BOOL "")

set(CMake_TEST_TLS_VERIFY_URL "https://gitlab.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERIFY_URL_BAD "https://badtls-expired.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERSION "1.2" CACHE STRING "")
set(CMake_TEST_TLS_VERSION_URL_BAD "https://tls-v1-1.badssl.com:1011" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
