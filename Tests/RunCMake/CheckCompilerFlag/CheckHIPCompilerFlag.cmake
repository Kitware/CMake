
enable_language (HIP)
include(CheckCompilerFlag)

check_compiler_flag(HIP "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid HIP compile flag didn't fail.")
endif()

check_compiler_flag(HIP "-DFOO" SHOULD_WORK)
if(NOT SHOULD_WORK)
  message(SEND_ERROR "${CMAKE_HIP_COMPILER_ID} compiler flag '-DFOO' check failed")
endif()
