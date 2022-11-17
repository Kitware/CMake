enable_language (OBJCXX)
include(CheckSourceCompiles)

set(OBJCXX 1) # test that this is tolerated

check_source_compiles(OBJCXX [[
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
