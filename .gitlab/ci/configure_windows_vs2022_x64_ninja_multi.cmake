if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_IAR_TOOLCHAINS "$ENV{CI_PROJECT_DIR}/.gitlab/iar" CACHE PATH "")
  set(CMake_TEST_ISPC "ON" CACHE STRING "")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_msvc_cxx_modules_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_msvc_common.cmake")
