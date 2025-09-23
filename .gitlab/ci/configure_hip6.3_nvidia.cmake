set(CMake_TEST_HIP "nvidia" CACHE BOOL "")
set(CMake_TEST_HIP_STANDARDS "98;11;14;17" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
