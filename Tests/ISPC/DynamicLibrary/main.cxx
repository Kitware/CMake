

#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

IMPORT int simple();
int extra();

int main()
{
  extra();
  simple();
  return 0;
}
