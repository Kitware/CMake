set(CMake_TEST_FindBLAS "Intel10_64lp;Intel10_64lp.gcc" CACHE STRING "")
set(CMake_TEST_FindBLAS_STATIC "Intel10_64lp;Intel10_64lp.gcc" CACHE STRING "")
set(CMake_TEST_FindLAPACK "Intel10_64lp;Intel10_64lp.gcc" CACHE STRING "")
set(CMake_TEST_FindLAPACK_STATIC "Intel10_64lp;Intel10_64lp.gcc" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
