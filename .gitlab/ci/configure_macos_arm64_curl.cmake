# Build with our vendored curl instead of the default system version.
set(CMAKE_USE_SYSTEM_CURL "OFF" CACHE BOOL "")

set(CMake_TEST_TLS_VERIFY_URL "https://gitlab.kitware.com" CACHE STRING "")
set(CMake_TEST_TLS_VERIFY_URL_BAD "https://badtls-expired.kitware.com" CACHE STRING "")

# Test that our vendored curl accepts CURL_SSLVERSION_TLSv1_3.  It is passed
# through to Secure Transport, but macOS does not actually enforce it.
set(CMake_TEST_TLS_VERSION "1.3" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
