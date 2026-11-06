#include <iostream>

int main()
{
  bool b = false;
  if (b) {
    // This line never runs, so coverage reports it as missed.
    std::cout << "this shouldn't happen..." << std::endl;
  }

  std::cout << "this test fails" << std::endl;
  return 1;
}
