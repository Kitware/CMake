
enable_language (HIP)
include(CheckSourceCompiles)

check_source_compiles(HIP "I don't build" SHOULD_FAIL)
if(SHOULD_FAIL)
  message(SEND_ERROR "invalid HIP source didn't fail.")
endif()

check_source_compiles(HIP [=[
  #include <vector>
  __device__ int d_func() { }
  int main() {
    return 0;
  }
]=]
 SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid HIP source.")
endif()

check_source_compiles(HIP "void l(char const (&x)[2]){}; int main() { l(\"\\n\"); return 0;}"
 SHOULD_BUILD_COMPLEX)

if(NOT SHOULD_BUILD_COMPLEX)
  message(SEND_ERROR "Test fail for valid HIP complex source.")
endif()
