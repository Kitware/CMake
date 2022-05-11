
enable_language (CXX)
include(CheckCompilerFlag)

set(CXX 1) # test that this is tolerated

# test that the check uses an isolated locale
set(_env_LC_ALL "${LC_ALL}")
set(ENV{LC_ALL} "BAD")

check_compiler_flag(CXX "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid CXX compile flag didn't fail.")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|LCC|Clang" AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
  check_compiler_flag(CXX "-x c++" SHOULD_WORK)
  if(NOT SHOULD_WORK)
    message(SEND_ERROR "${CMAKE_CXX_COMPILER_ID} compiler flag '-x c++' check failed")
  endif()
endif()

if(NOT "$ENV{LC_ALL}" STREQUAL "BAD")
  message(SEND_ERROR "ENV{LC_ALL} was not preserved by check_compiler_flag")
endif()
set(ENV{LC_ALL} ${_env_LC_ALL})
