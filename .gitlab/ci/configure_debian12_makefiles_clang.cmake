if (NOT "$ENV{CMAKE_CI_NIGHTLY}" STREQUAL "")
  set(CMake_TEST_TICLANG_TOOLCHAINS "$ENV{CI_PROJECT_DIR}/.gitlab/ticlang" CACHE PATH "")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
