set(CMake_TEST_CUDA "Clang" CACHE STRING "")
set(CMake_TEST_CUDA_STANDARDS "03;11;14;17;20;23" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
