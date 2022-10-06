enable_language (OBJC)
include(CheckOBJCSourceCompiles)

set(OBJC 1) # test that this is tolerated

check_objc_source_compiles("I don't build in Objective-C" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid OBJC source didn't fail.")
endif()

check_objc_source_compiles([[
  #import <Foundation/Foundation.h>
  int main() {
    NSObject *foo;
    return 0;
  }
]] SHOULD_BUILD)

if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid OBJC source.")
endif()
