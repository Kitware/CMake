
enable_language (C)
include(CheckCompilerFlag)

check_compiler_flag(C "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid C compile flag didn't fail.")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT "x${CMAKE_C_SIMULATE_ID}" STREQUAL "xMSVC")
  check_compiler_flag(C "-x c" SHOULD_WORK)
  if(NOT SHOULD_WORK)
    message(SEND_ERROR "${CMAKE_C_COMPILER_ID} compiler flag '-x c' check failed")
  endif()
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  check_compiler_flag(C "-frtti" SHOULD_FAIL_RTTI)
  if(SHOULD_FAIL_RTTI)
    message(SEND_ERROR "${CMAKE_C_COMPILER_ID} compiler flag '-frtti' check passed but should have failed")
  endif()
endif()
