
enable_language (ISPC)
include(CheckCompilerFlag)

check_compiler_flag(ISPC "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid ISPC compile flag didn't fail.")
endif()

check_compiler_flag(ISPC "--woff" SHOULD_WORK)
if(NOT SHOULD_WORK)
  message(SEND_ERROR "${CMAKE_ISPC_COMPILER_ID} compiler flag '--woff' check failed")
endif()
