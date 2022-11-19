
enable_language (C)
include(CheckCCompilerFlag)

set(C 1) # test that this is tolerated

if(NOT CMAKE_C_COMPILER_ID STREQUAL "PathScale")
  set(DD --)
endif()

check_c_compiler_flag("${DD}-_this_is_not_a_flag_" C_BOGUS_FLAG)
if(C_BOGUS_FLAG)
  message(SEND_ERROR "CHECK_C_COMPILER_FLAG() succeeded, but should have failed")
endif()
unset(C_BOGUS_FLAG CACHE)
if(DEFINED C_BOGUS_FLAG)
  # Verify that CHECK_C_COMPILER_FLAG didn't construct a normal variable
  message(SEND_ERROR "CHECK_C_COMPILER_FLAG shouldn't construct C_BOGUS_FLAG as a normal variable")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "LCC")
  unset(C_STRICT_PROTOTYPES CACHE)
  CHECK_C_COMPILER_FLAG("-Werror;-Wstrict-prototypes" C_STRICT_PROTOTYPES)
  if(NOT C_STRICT_PROTOTYPES)
    message(SEND_ERROR "CHECK_C_COMPILER_FLAG failed -Werror -Wstrict-prototypes")
  endif()
endif()
