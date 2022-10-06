
enable_language (C)
include(CheckCSourceCompiles)

set(C 1) # test that this is tolerated

check_c_source_compiles("I don't build" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid C source didn't fail.")
endif()

check_c_source_compiles("int main() {return 0;}" SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid C source.")
endif()
