
#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

void __global__ file2_kernel(int x, int* r)
{
  *r = -x;
}

EXPORT int file2_launch_kernel(int x)
{
  int* r;
  cudaMallocManaged(&r, sizeof(int));

  file2_kernel<<<1, 1>>>(x, r);
  cudaDeviceSynchronize();

  auto result = *r;
  cudaFree(r);

  return result;
}
