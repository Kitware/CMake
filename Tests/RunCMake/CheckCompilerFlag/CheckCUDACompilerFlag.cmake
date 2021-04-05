
enable_language (CUDA)
include(CheckCompilerFlag)

set(CUDA 1) # test that this is tolerated

check_compiler_flag(CUDA "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid CUDA compile flag didn't fail.")
endif()

check_compiler_flag(CUDA "-DFOO" SHOULD_WORK)
if(NOT SHOULD_WORK)
  message(SEND_ERROR "${CMAKE_CUDA_COMPILER_ID} compiler flag '-DFOO' check failed")
endif()
