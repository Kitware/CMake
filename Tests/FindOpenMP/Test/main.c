#include <omp.h>
int main(void)
{
#ifdef _OPENMP
  omp_get_num_threads();
#else
#  error "_OPENMP not defined!"
#endif
  return 0;
}
