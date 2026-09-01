set(CMake_TEST_CUDA "Clang" CACHE STRING "")
set(CMake_TEST_CUDA_ARCH "75" CACHE STRING "")
set(CMake_TEST_CUDA_STANDARDS "03;11;14;17;20;23" CACHE STRING "")
set(CMake_TEST_FindOpenMP "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenMP_CUDA "ON" CACHE BOOL "")

set(configure_no_sccache ON)

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
