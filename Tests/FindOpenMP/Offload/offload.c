#include <omp.h>

#ifdef __cplusplus
extern "C"
#endif
  int
  offload_check(void)
{
  int i;
  int on_host = 1;
  int sum = 0;
#pragma omp target map(tofrom : on_host, sum)
  {
    on_host = omp_is_initial_device();
    for (i = 1; i <= 10; ++i) {
      sum += i;
    }
  }
  return on_host || sum != 55;
}
