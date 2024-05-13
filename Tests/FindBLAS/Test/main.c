#include <assert.h>
#include <stdint.h>
#include <string.h>

#if BLA_SIZEOF_INTEGER == 4
typedef int32_t blas_int;
#elif BLA_SIZEOF_INTEGER == 8
typedef int64_t blas_int;
#else
#  error BLA_SIZEOF_INTEGER is not declared!
#endif

// declare what parts of the blas C-API we need
void dswap_(blas_int* N, double* X, blas_int* incX, double* Y, blas_int* incY);

int main(void)
{
  double x[4] = { 1, 2, 3, 4 };
  double y[4] = { 8, 7, 7, 6 };
  blas_int N = 4;
  blas_int incX = 1;
  blas_int incY = 1;
  dswap_(&N, x, &incX, y, &incY);
  return 0;
}
