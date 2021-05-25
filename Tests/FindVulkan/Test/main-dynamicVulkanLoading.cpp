#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <iostream>

#include <vulkan/vulkan.hpp>

using namespace std;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

int main()
{
  // catch exceptions
  // (vulkan.hpp functions throws if they fail)
  try {

    // initialize dynamic dispatcher
    vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
      dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    // Vulkan instance
    vk::UniqueInstance instance =
      vk::createInstanceUnique(vk::InstanceCreateInfo{
        vk::InstanceCreateFlags(), // flags
        &(const vk::ApplicationInfo&)vk::ApplicationInfo{
          "CMake Test application", // application name
          VK_MAKE_VERSION(0, 0, 0), // application version
          "CMake Test Engine",      // engine name
          VK_MAKE_VERSION(0, 0, 0), // engine version
          VK_API_VERSION_1_0,       // api version
        },
        0,       // enabled layer count
        nullptr, // enabled layer names
        0,       // enabled extension count
        nullptr, // enabled extension names
      });
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance.get());

    // catch exceptions
  } catch (vk::Error& e) {
    cout << "Failed because of Vulkan exception: " << e.what() << endl;
  } catch (exception& e) {
    cout << "Failed because of exception: " << e.what() << endl;
  } catch (...) {
    cout << "Failed because of unspecified exception." << endl;
  }

  // We can't assert in this code because in general vk::createInstanceUnique
  // might throw if no driver is found - but if we get here, FindVulkan is
  // working

  return 0;
}
