set(CMake_TEST_CUDA "NVIDIA" CACHE STRING "")
set(CMake_TEST_CUDA_CUPTI "ON" CACHE STRING "")

set(configure_no_sccache 1)

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
