#include <assert.h>
#include <string.h>

// declare what parts of the blas C-API we need
void dswap_(int* N, double* X, int* incX, double* Y, int* incY);

int main()
{
  double x[4] = { 1, 2, 3, 4 };
  double y[4] = { 8, 7, 7, 6 };
  int N = 4;
  int incX = 1;
  int incY = 1;
  dswap_(&N, x, &incX, y, &incY);
  return 0;
}
