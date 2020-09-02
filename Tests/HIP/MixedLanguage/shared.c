

#ifdef _WIN32
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT
#endif

EXPORT int shared_c_func(int x)
{
  return -x;
}
