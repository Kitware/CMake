
enable_language (C)
include(CheckCompilerFlag)

set(C 1) # test that this is tolerated

# test that the check uses an isolated locale
set(_env_LC_ALL "${LC_ALL}")
set(ENV{LC_ALL} "BAD")

check_compiler_flag(C "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid C compile flag didn't fail.")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|LCC|Clang" AND NOT "x${CMAKE_C_SIMULATE_ID}" STREQUAL "xMSVC")
  check_compiler_flag(C "-x c" SHOULD_WORK)
  if(NOT SHOULD_WORK)
    message(SEND_ERROR "${CMAKE_C_COMPILER_ID} compiler flag '-x c' check failed")
  endif()
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU") # LCC C compiler silently ignore -frtti instead of failing, so skip it here.
  check_compiler_flag(C "-frtti" SHOULD_FAIL_RTTI)
  if(SHOULD_FAIL_RTTI)
    message(SEND_ERROR "${CMAKE_C_COMPILER_ID} compiler flag '-frtti' check passed but should have failed")
  endif()
endif()

if(NOT "$ENV{LC_ALL}" STREQUAL "BAD")
  message(SEND_ERROR "ENV{LC_ALL} was not preserved by check_compiler_flag")
endif()
set(ENV{LC_ALL} ${_env_LC_ALL})
