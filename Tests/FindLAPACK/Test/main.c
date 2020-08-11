#include <assert.h>
#include <string.h>

// declare what parts of the lapack C-API we need
void dgesv_(int*, int*, double*, int*, int*, double*, int*, int*);

int main()
{
  double A[8] = {
    0, 1, 2, 3, 4, 5, 6, 7,
  };
  double B[2] = { 0, 5 };
  int ipiv[2] = { 0, 0 };
  int info = 0;

  int dim = 2;
  int numCols = 1;
  dgesv_(&dim, &numCols, A, &dim, ipiv, B, &dim, &info);
  return 0;
}
