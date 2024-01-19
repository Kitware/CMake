#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __attribute__((__visibility__("default")))
#endif

__global__ void b1()
{
}

EXPORT int bar()
{
  b1<<<1, 1>>>();
  return 0;
}
