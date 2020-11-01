
#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#  define EXPORT __declspec(dllexport)
#else
#  define IMPORT
#  define EXPORT
#endif

IMPORT int curand_main();
IMPORT int nppif_main();

EXPORT int mixed_version()
{
  return curand_main() == 0 && nppif_main() == 0;
}
