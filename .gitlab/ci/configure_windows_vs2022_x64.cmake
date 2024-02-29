if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_ANDROID_VS17 ON CACHE BOOL "")
endif()

set(CMake_TEST_MODULE_COMPILATION "named,partitions,internal_partitions,shared" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_msvc_cxx_modules_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_vs_common.cmake")
