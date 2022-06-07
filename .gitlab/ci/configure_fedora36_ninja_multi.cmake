if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_ISPC "ON" CACHE STRING "")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
