
enable_language (C)
include(CheckSourceCompiles)

set(C 1) # test that this is tolerated

check_source_compiles(C "I don't build" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid C source didn't fail.")
endif()

check_source_compiles(C "int main() {return 0;}" SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid C source.")
endif()
