
#include <vector>

void vecaddgpu(float* r, float* a, float* b, std::size_t n)
{
#pragma acc kernels loop present(r, a, b)
  for (std::size_t i = 0; i < n; ++i)
    r[i] = a[i] + b[i];
}

int main(int, char*[])
{
  const std::size_t n = 100000; /* vector length */
  std::vector<float> a(n);      /* input vector 1 */
  std::vector<float> b(n);      /* input vector 2 */
  std::vector<float> r(n);      /* output vector */
  std::vector<float> e(n);      /* expected output values */

  for (std::size_t i = 0; i < n; ++i) {
    a[i] = static_cast<float>(i + 1);
    b[i] = static_cast<float>(1000 * i);
  }

  /* compute on the GPU */
  auto a_ptr = a.data();
  auto b_ptr = b.data();
  auto r_ptr = r.data();
#pragma acc data copyin(a_ptr [0:n], b_ptr [0:n]) copyout(r_ptr [0:n])
  {
    vecaddgpu(r_ptr, a_ptr, b_ptr, n);
  }
  /* compute on the host to compare */
  for (std::size_t i = 0; i < n; ++i)
    e[i] = a[i] + b[i];
  /* compare results */
  int errs = 0;
  for (std::size_t i = 0; i < n; ++i) {
    if (r[i] != e[i]) {
      ++errs;
    }
  }
  return errs;
}
