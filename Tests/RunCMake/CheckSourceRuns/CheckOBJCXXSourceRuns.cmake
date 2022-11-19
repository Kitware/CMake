enable_language (OBJCXX)
include(CheckOBJCXXSourceRuns)

set(OBJCXX 1) # test that this is tolerated

check_objcxx_source_runs("int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "check_objcxx_source_runs succeeded, but should have failed.")
endif()

check_objcxx_source_runs([[
  #include <vector>
  #import <Foundation/Foundation.h>
  int main() {
    std::vector<int> v;
    NSObject *foo;
    return 0;
  }
]] SHOULD_BUILD)


if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for OBJCXX source.")
endif()
