
#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#  define EXPORT __declspec(dllexport)
#else
#  define IMPORT
#  define EXPORT
#endif

int curand_main();
int nppif_main();

EXPORT int shared_version()
{
  return curand_main() == 0 && nppif_main() == 0;
}
