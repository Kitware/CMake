cmake_policy(SET CMP0157 NEW)
enable_language(Swift)
include(CheckSourceCompiles)

set(Swift 1) # test that this is tolerated

check_source_compiles(Swift "baz()" SHOULD_FAIL)

if(SHOULD_FAIL)
  message(SEND_ERROR "invalid Swift source didn't fail.")
endif()

check_source_compiles(Swift "print(\"Hello, CMake\")" SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test failed for valid Swift source.")
endif()
