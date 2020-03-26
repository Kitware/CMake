
#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
IMPORT int shared_version();
int static_version()
{
  return 0;
}
int mixed_version()
{
  return 0;
}
#else
int shared_version();
int static_version();
int mixed_version();
#endif

int main()
{
  return mixed_version() == 0 && shared_version() == 0 &&
    static_version() == 0;
}
