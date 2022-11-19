enable_language (OBJC)
include(CheckOBJCSourceRuns)

set(OBJC 1) # test that this is tolerated

check_objc_source_runs("int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "check_objc_source_runs succeeded, but should have failed.")
endif()

check_objc_source_runs([[
  #import <Foundation/Foundation.h>
  int main() {
    NSObject *foo;
    return 0;
  }
]] SHOULD_BUILD)

if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid OBJC source.")
endif()
