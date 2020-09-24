enable_language (OBJC)
include(CheckSourceRuns)

check_source_runs(OBJC [[
  #import <Foundation/Foundation.h>
  int main() {
    NSObject *foo;
    return 0;
  }
]] SHOULD_BUILD)

if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid OBJC source.")
endif()
