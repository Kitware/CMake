enable_language (OBJC)
include(CheckCompilerFlag)

check_compiler_flag(OBJC "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid OBJC compile flag didn't fail.")
endif()

check_compiler_flag(OBJC "-Wall" SHOULD_WORK)
if(NOT SHOULD_WORK)
  message(SEND_ERROR "${CMAKE_OBJC_COMPILER_ID} compiler flag '-Wall' check failed")
endif()
