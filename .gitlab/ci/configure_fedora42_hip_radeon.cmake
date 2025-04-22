set(CMake_TEST_HIP "amd" CACHE BOOL "")
set(CMake_TEST_HIP_STANDARDS "98;11;14;17;20;23;26" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
