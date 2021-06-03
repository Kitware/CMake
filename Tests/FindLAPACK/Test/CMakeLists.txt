cmake_minimum_required(VERSION 3.4)
project(TestFindLAPACK C)
include(CTest)

find_package(LAPACK REQUIRED)

add_executable(test_tgt main.c)
target_link_libraries(test_tgt LAPACK::LAPACK)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_link_libraries(test_var PRIVATE ${LAPACK_LIBRARIES})
add_test(NAME test_var COMMAND test_var)

if(BLA_VENDOR STREQUAL "Intel10_64lp")
  if(NOT LAPACK_LIBRARIES MATCHES "^(-Wl,--start-group;)?[^;]*mkl_intel_lp64")
    message(FATAL_ERROR "LAPACK_LIBRARIES does not start in mkl_intel_lp64:\n ${LAPACK_LIBRARIES}")
  endif()
endif()
