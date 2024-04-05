set(CMake_TEST_CUDA "NVIDIA" CACHE STRING "")
set(CMake_TEST_CUDA_CUPTI "ON" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
