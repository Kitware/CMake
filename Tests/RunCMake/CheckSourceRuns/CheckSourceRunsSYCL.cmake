
set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language (SYCL)
include(CheckSourceRuns)

set(SYCL 1) # test that this is tolerated

check_source_runs(SYCL "int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "SYCL check_source_runs succeeded, but should have failed.")
endif()

check_source_runs(SYCL
[=[
  #include <vector>
  #include <sycl/sycl.hpp>
  void fake_function();
  int main() {
    return 0;
  }
]=]
 SHOULD_RUN)
if(NOT SHOULD_RUN)
  message(SEND_ERROR "SYCL check_source_runs failed for valid SYCL executable.")
endif()
