set(CMAKE_Fortran_COMPILER "/usr/bin/flang-new" CACHE FILEPATH "")
set(CMAKE_Fortran_COMPILER_ID "LLVMFlang" CACHE STRING "")
set(CMAKE_Fortran_COMPILER_SUPPORTS_F90 "1" CACHE BOOL "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
