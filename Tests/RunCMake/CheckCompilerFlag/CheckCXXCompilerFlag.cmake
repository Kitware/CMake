
enable_language (CXX)
include(CheckCompilerFlag)

set(CXX 1) # test that this is tolerated

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
