enable_language (Fortran)
include(CheckCompilerFlag)

set(Fortran 1) # test that this is tolerated

check_compiler_flag(Fortran "-_this_is_not_a_flag_" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid Fortran compile flag didn't fail.")
endif()

if(CMAKE_Fortran_COMPILER_ID STREQUAL "GNU|LCC")
  check_compiler_flag(Fortran "-Wall" SHOULD_WORK)
  if(NOT SHOULD_WORK)
    message(SEND_ERROR "${CMAKE_Fortran_COMPILER_ID} compiler flag '-Wall' check failed")
  endif()
endif()
