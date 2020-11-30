enable_language (OBJCXX)
include(CheckCompilerFlag)

set(OBJCXX 1) # test that this is tolerated

check_compiler_flag(OBJCXX "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid OBJCXX compile flag didn't fail.")
endif()

check_compiler_flag(OBJCXX "-Wall" SHOULD_WORK)
if(NOT SHOULD_WORK)
  message(SEND_ERROR "${CMAKE_OBJCXX_COMPILER_ID} compiler flag '-Wall' check failed")
endif()
