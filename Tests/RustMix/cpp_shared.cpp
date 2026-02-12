#include <iostream>

extern "C" void cpp_shared_greet()
{
  std::cout << "Hello from a C++ shader library" << std::endl;
}
