
enable_language (CXX)
include(CheckCXXCompilerFlag)

set(CXX 1) # test that this is tolerated

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "PathScale")
  set(DD --)
endif()

check_cxx_compiler_flag("${DD}-_this_is_not_a_flag_" CXX_BOGUS_FLAG)
if(CXX_BOGUS_FLAG)
  message(SEND_ERROR "CHECK_CXX_COMPILER_FLAG() succeeded, but should have failed")
endif()
unset(CXX_BOGUS_FLAG CACHE)
if(DEFINED CXX_BOGUS_FLAG)
  # Verify that CHECK_CXX_COMPILER_FLAG didn't construct a normal variable
  message(SEND_ERROR "CHECK_CXX_COMPILER_FLAG shouldn't construct CXX_BOGUS_FLAG as a normal variable")
endif()
