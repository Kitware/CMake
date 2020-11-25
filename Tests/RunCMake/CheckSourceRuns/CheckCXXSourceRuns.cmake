
enable_language (CXX)
include(CheckSourceRuns)

set(CXX 1) # test that this is tolerated

check_source_runs(CXX "int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "CXX check_source_runs succeeded, but should have failed.")
endif()

check_source_runs(CXX
[=[
  #include <vector>
  int main() {
    return 0;
  }
]=]
 SHOULD_RUN)
if(NOT SHOULD_RUN)
  message(SEND_ERROR "CXX check_source_runs failed for valid C executable.")
endif()
