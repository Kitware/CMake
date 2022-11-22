
#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

IMPORT int shared_version();

#ifdef HAS_STATIC_VERSION
IMPORT int static_version();
#else
int static_version()
{
  return 0;
}
#endif

#ifdef HAS_MIXED_VERSION
IMPORT int mixed_version();
#else
int mixed_version()
{
  return 0;
}
#endif

int main()
{
  return mixed_version() == 0 && shared_version() == 0 &&
    static_version() == 0;
}
