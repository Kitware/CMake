

enable_language (Fortran)
include(CheckSourceCompiles)

set(Fortran 1) # test that this is tolerated

# lfortran < 1.24 cannot handle long file names.
if(CMAKE_Fortran_COMPILER_ID STREQUAL "LCC" AND CMAKE_Fortran_COMPILER_VERSION VERSION_LESS "1.24")
  string(LENGTH "${CMAKE_CURRENT_BINARY_DIR}" _CCBD_LEN)
  if(_CCBD_LEN GREATER_EQUAL 35)
    return()
  endif()
endif()

check_source_compiles(Fortran [=[
      PROGRAM TEST_HAVE_PRINT
        PRINT *, 'Hello'
      END
]=] SHOULD_BUILD)

if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid Fortran source.")
endif()
