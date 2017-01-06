
#include <iostream>

int static_func(int);
int file1_sq_func(int);

void test_functions()
{
  file1_sq_func(static_func(42));
}

int main(int argc, char** argv)
{
  test_functions();
  std::cout
    << "this executable doesn't use cuda code, just call methods defined"
    << std::endl;
  std::cout << "in object files that have cuda code" << std::endl;
  return 0;
}
