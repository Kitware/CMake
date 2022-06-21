#include <iostream>

#include <volk/volk.h>

int main()
{
  if (volkInitialize() != VK_SUCCESS) {
    std::cout << "volk initialization success!" << std::endl;
  } else {
    std::cout << "volk initialization failure!" << std::endl;
  }

  return 0;
}
