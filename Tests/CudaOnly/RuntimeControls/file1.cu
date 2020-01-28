
#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

void __global__ file1_kernel(int x, int& r)
{
  r = -x;
}

EXPORT int file1_launch_kernel(int x)
{
  int r = 0;
  file1_kernel<<<1, 1>>>(x, r);
  return r;
}
