
enable_language (CUDA)
include(CheckSourceCompiles)

set(CUDA 1) # test that this is tolerated

check_source_compiles(CUDA "I don't build" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid CUDA source didn't fail.")
endif()

check_source_compiles(CUDA [=[
  #include <vector>
  __device__ int d_func() { }
  int main() {
    return 0;
  }
]=]
 SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid CUDA source.")
endif()

check_source_compiles(CUDA "void l(char const (&x)[2]){}; int main() { l(\"\\n\"); return 0;}"
 SHOULD_BUILD_COMPLEX)

if(NOT SHOULD_BUILD_COMPLEX)
  message(SEND_ERROR "Test fail for valid CUDA complex source.")
endif()
