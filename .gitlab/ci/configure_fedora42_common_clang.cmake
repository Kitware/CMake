set(CMAKE_Fortran_COMPILER "/usr/bin/flang-new" CACHE FILEPATH "")
set(CMAKE_Fortran_COMPILER_ID "LLVMFlang" CACHE STRING "")
set(CMAKE_Fortran_COMPILER_SUPPORTS_F90 "1" CACHE BOOL "")

set(CMake_TEST_C_STANDARDS "90;99;11;17;23" CACHE STRING "")
set(CMake_TEST_CXX_STANDARDS "98;11;14;17;20;23;26" CACHE STRING "")

set(CMake_TEST_FindOpenACC "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenACC_C "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenACC_CXX "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenACC_Fortran "OFF" CACHE BOOL "") # flang-new fails producing LLVM IR
set(CMake_TEST_FindOpenMP_C "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenMP_CXX "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenMP_Fortran "ON" CACHE BOOL "")
set(CMake_TEST_FindOpenMP "ON" CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
