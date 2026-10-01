set(CMake_TEST_ASM_NASM "ON" CACHE BOOL "")
if(NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_CPACK_APK "ON" CACHE STRING "")
  set(CMake_TEST_CPACK_APK_SDK_ROOT "$ENV{ANDROID_HOME}" CACHE FILEPATH "")
  set(CMake_TEST_CPACK_APK_TOOLCHAIN_FILE "$ENV{ANDROID_NDK_ROOT}/build/cmake/android.toolchain.cmake" CACHE FILEPATH "")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/configure_debian13_ninja_common.cmake")
set(CMake_TEST_UseSWIG "ON" CACHE BOOL "")
