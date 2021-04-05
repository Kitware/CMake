

enable_language (Fortran)
include(CheckSourceCompiles)

set(Fortran 1) # test that this is tolerated

check_source_compiles(Fortran [=[
      PROGRAM TEST_HAVE_PRINT
        PRINT *, 'Hello'
      END
]=] SHOULD_BUILD)

if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid Fortran source.")
endif()
