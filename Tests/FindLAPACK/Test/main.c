#include <assert.h>
#include <string.h>

typedef int blas_int;

// declare what parts of the lapack C-API we need
void dgesv_(blas_int*, blas_int*, double*, blas_int*, blas_int*, double*,
            blas_int*, blas_int*);

int main()
{
  double A[8] = {
    0, 1, 2, 3, 4, 5, 6, 7,
  };
  double B[2] = { 0, 5 };
  blas_int ipiv[2] = { 0, 0 };
  blas_int info = 0;

  blas_int dim = 2;
  blas_int numCols = 1;
  dgesv_(&dim, &numCols, A, &dim, ipiv, B, &dim, &info);
  return 0;
}
