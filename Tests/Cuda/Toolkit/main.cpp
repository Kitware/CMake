// Only thing we care about is that these headers are found
#include <cuda.h>
#include <cuda_runtime_api.h>
#if CUDA_VERSION >= 11040
#  include <nv/target>
#endif
#include <thrust/version.h>

int main()
{
  return 0;
}
