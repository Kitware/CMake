
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language (SYCL)
include(CheckCompilerFlag)

set(SYCL 1) # test that this is tolerated

check_compiler_flag(SYCL "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid SYCL compile flag didn't fail.")
endif()

check_compiler_flag(SYCL "-DFOO" SHOULD_WORK)
if(NOT SHOULD_WORK)
  message(SEND_ERROR "${CMAKE_SYCL_COMPILER_ID} compiler flag '-DFOO' check failed")
endif()
