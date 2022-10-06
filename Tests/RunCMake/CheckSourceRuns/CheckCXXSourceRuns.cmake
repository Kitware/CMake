
enable_language (CXX)
include(CheckCXXSourceRuns)

set(CXX 1) # test that this is tolerated

check_cxx_source_runs("int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "check_cxx_source_runs succeeded, but should have failed.")
endif()

check_cxx_source_runs(
[=[
  #include <vector>
  int main() {
    return 0;
  }
]=]
 SHOULD_RUN)
if(NOT SHOULD_RUN)
  message(SEND_ERROR "check_cxx_source_runs failed for valid C executable.")
endif()
