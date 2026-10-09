
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language (SYCL)
include(CheckSourceCompiles)

set(SYCL 1) # test that this is tolerated

check_source_compiles(SYCL "I don't build" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid SYCL source didn't fail.")
endif()

check_source_compiles(SYCL [=[
  #include <vector>
  #include <sycl/sycl.hpp>
  int d_func() { return 0; }
  int main() {
    return 0;
  }
]=]
 SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid SYCL source.")
endif()

check_source_compiles(SYCL "void l(char const (&x)[2]){}; int main() { l(\"\\n\"); return 0;}"
 SHOULD_BUILD_COMPLEX)

if(NOT SHOULD_BUILD_COMPLEX)
  message(SEND_ERROR "Test fail for valid SYCL complex source.")
endif()
