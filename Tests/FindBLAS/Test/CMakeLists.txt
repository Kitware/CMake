cmake_minimum_required(VERSION 3.5)
project(TestFindBLAS C)
include(CTest)

find_package(BLAS REQUIRED)

if(NOT BLA_SIZEOF_INTEGER)
  set(BLA_SIZEOF_INTEGER 4)
endif()

add_executable(test_tgt main.c)
target_link_libraries(test_tgt BLAS::BLAS)
target_compile_definitions(test_tgt PUBLIC BLA_SIZEOF_INTEGER=${BLA_SIZEOF_INTEGER})
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_link_libraries(test_var PRIVATE ${BLAS_LIBRARIES})
target_compile_definitions(test_var PUBLIC BLA_SIZEOF_INTEGER=${BLA_SIZEOF_INTEGER})
add_test(NAME test_var COMMAND test_var)

if((BLA_VENDOR STREQUAL "Intel10_64lp") OR
   (BLA_VENDOR STREQUAL "All" AND EXPECT_All STREQUAL "Intel10_64lp"))
  if(NOT BLAS_LIBRARIES MATCHES "^(-Wl,--start-group;)?[^;]*mkl_intel_lp64")
    message(FATAL_ERROR "BLAS_LIBRARIES does not start in mkl_intel_lp64:\n ${BLAS_LIBRARIES}")
  endif()
elseif((BLA_VENDOR STREQUAL "Intel10_64ilp") OR
   (BLA_VENDOR STREQUAL "All" AND EXPECT_All STREQUAL "Intel10_64ilp"))
  if(NOT BLAS_LIBRARIES MATCHES "^(-Wl,--start-group;)?[^;]*mkl_intel_ilp64")
    message(FATAL_ERROR "BLAS_LIBRARIES does not start in mkl_intel_ilp64:\n ${BLAS_LIBRARIES}")
  endif()
endif()
