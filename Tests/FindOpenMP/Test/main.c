#include <omp.h>
int main(void)
{
#ifdef _OPENMP
  omp_get_num_threads();
#elif !defined(__CUDA_ARCH__) && !defined(__HIP_DEVICE_COMPILE__)
#  error "_OPENMP not defined!"
#endif
  return 0;
}
