set(blas_lapack_cases
  all=Intel10_64lp
    static=0 All Intel10_64lp compiler=gcc Intel10_64lp compiler=
    static=1 All Intel10_64lp compiler=gcc Intel10_64lp compiler=
  )
set(CMake_TEST_FindBLAS "${blas_lapack_cases}" CACHE STRING "")
set(CMake_TEST_FindLAPACK "${blas_lapack_cases}" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_external_test.cmake")
