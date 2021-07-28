
#include <iostream>

#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

int static_hip11_func(int);
IMPORT int shared_hip11_func(int);

void test_functions()
{
  static_hip11_func(int(42));
  shared_hip11_func(int(42));
}

int main(int argc, char** argv)
{
  test_functions();
  return 0;
}
