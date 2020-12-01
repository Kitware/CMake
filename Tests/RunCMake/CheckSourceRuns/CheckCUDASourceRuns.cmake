
enable_language (CUDA)
include(CheckSourceRuns)

set(CUDA 1) # test that this is tolerated

check_source_runs(CUDA "int main() {return 2;}" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "CUDA check_source_runs succeeded, but should have failed.")
endif()

check_source_runs(CUDA
[=[
  #include <vector>
  __device__ __host__ void fake_function();
  __host__ int main() {
    return 0;
  }
]=]
 SHOULD_RUN)
if(NOT SHOULD_RUN)
  message(SEND_ERROR "CUDA check_source_runs failed for valid CUDA executable.")
endif()
