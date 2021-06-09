
enable_language (HIP)
include(CheckSourceRuns)

check_source_runs(HIP "int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "HIP check_source_runs succeeded, but should have failed.")
endif()

check_source_runs(HIP
[=[
  #include <vector>
  __device__ __host__ void fake_function();
  __host__ int main() {
    return 0;
  }
]=]
 SHOULD_RUN)
if(NOT SHOULD_RUN)
  message(SEND_ERROR "HIP check_source_runs failed for valid HIP executable.")
endif()
