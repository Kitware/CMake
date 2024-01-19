#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

IMPORT int foo();
IMPORT int bar();

int main(int argc, char**)
{
  return foo() && bar();
}
