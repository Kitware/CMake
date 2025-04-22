set(CMake_TEST_CUDA "NVIDIA" CACHE STRING "")
set(CMake_TEST_CUDA_ARCH "30" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
