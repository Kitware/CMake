#include <assert.h>
#include <string.h>

// declare what parts of the blas C-API we need
void cblas_dswap(const int N, double* X, const int incX, double* Y,
                 const int incY);

int main()
{
  double x[4] = { 1, 2, 3, 4 };
  double y[4] = { 8, 7, 7, 6 };
  cblas_dswap(4, x, 1, y, 1);
  return 0;
}
