
enable_language (CXX)
include(CheckSourceCompiles)

set(CXX 1) # test that this is tolerated

check_source_compiles(CXX "I don't build" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid CXX source didn't fail.")
endif()

check_source_compiles(CXX [=[
  #include <vector>
  int main() {
    return 0;
  }
]=]
 SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid CXX source.")
endif()

check_source_compiles(CXX "void l(char const (&x)[2]){}; int main() { l(\"\\n\"); return 0;}"
 SHOULD_BUILD_COMPLEX)

if(NOT SHOULD_BUILD_COMPLEX)
  message(SEND_ERROR "Test fail for valid CXX complex source.")
endif()
