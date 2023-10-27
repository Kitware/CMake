#include <stdio.h>
#include <stdlib.h>

void vecaddgpu(float* r, float* a, float* b, int n)
{
#pragma acc kernels loop present(r, a, b)
  for (int i = 0; i < n; ++i)
    r[i] = a[i] + b[i];
}

int main(void)
{
  int n = 100000; /* vector length */
  float* a;       /* input vector 1 */
  float* b;       /* input vector 2 */
  float* r;       /* output vector */
  float* e;       /* expected output values */
  int i, errs;

  a = (float*)malloc(n * sizeof(float));
  b = (float*)malloc(n * sizeof(float));
  r = (float*)malloc(n * sizeof(float));
  e = (float*)malloc(n * sizeof(float));
  for (i = 0; i < n; ++i) {
    a[i] = (float)(i + 1);
    b[i] = (float)(1000 * i);
  }
/* compute on the GPU */
#pragma acc data copyin(a [0:n], b [0:n]) copyout(r [0:n])
  {
    vecaddgpu(r, a, b, n);
  }
  /* compute on the host to compare */
  for (i = 0; i < n; ++i)
    e[i] = a[i] + b[i];
  /* compare results */
  errs = 0;
  for (i = 0; i < n; ++i) {
    if (r[i] != e[i]) {
      ++errs;
    }
  }
  return errs;
}
