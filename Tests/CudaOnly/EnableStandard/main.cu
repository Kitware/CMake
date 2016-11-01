
#include <iostream>

int static_cuda11_func(int);
int shared_cuda11_func(int);

void test_functions()
{
  static_cuda11_func( int(42) );
  shared_cuda11_func( int(42) );
}

int main(int argc, char **argv)
{
  test_functions();
  return 0;
}
