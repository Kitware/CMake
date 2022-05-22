#include <iostream>

#include <MoltenVK/vk_mvk_moltenvk.h>

int main()
{
  char mvk_version[256];
  char vk_version[256];
  vkGetVersionStringsMVK(mvk_version, sizeof(mvk_version), vk_version,
                         sizeof(vk_version));

  std::cout << "MoltenVK version: " << mvk_version << std::endl;
  std::cout << "Vulkan version: " << vk_version << std::endl;

  return 0;
}
