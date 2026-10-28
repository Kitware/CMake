if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_SYCL "ON" CACHE BOOL "")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
