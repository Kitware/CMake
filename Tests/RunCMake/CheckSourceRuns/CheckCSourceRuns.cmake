
enable_language (C)
include(CheckCSourceRuns)

set(C 1) # test that this is tolerated

check_c_source_runs("int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "check_c_source_runs succeeded, but should have failed.")
endif()

check_c_source_runs("int main() {return 0;}" SHOULD_RUN)
if(NOT SHOULD_RUN)
  message(SEND_ERROR "check_c_source_runs failed for valid C executable.")
endif()
