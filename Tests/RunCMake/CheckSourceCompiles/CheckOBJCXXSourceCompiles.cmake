enable_language (OBJCXX)
include(CheckOBJCXXSourceCompiles)

set(OBJCXX 1) # test that this is tolerated

check_objcxx_source_compiles("I don't build in Objective-C++" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid OBJCXX source didn't fail.")
endif()

check_objcxx_source_compiles([[
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
