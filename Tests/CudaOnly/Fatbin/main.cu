#include <iostream>
#include <string>
#include <vector>

#include <cuda.h>

#define GENERATED_HEADER(x) GENERATED_HEADER1(x)
#define GENERATED_HEADER1(x) <x>

static std::string input_paths = { FATBIN_FILE_PATHS };

int main()
{
  const std::string delimiter = "~_~";
  input_paths += delimiter;

  size_t end = 0;
  size_t previous_end = 0;
  std::vector<std::string> actual_paths;
  while ((end = input_paths.find(delimiter, previous_end)) !=
         std::string::npos) {
    actual_paths.emplace_back(
      input_paths.substr(previous_end, end - previous_end));
    previous_end = end + 3;
  }

  cuInit(0);
  int count = 0;
  cuDeviceGetCount(&count);
  if (count == 0) {
    std::cerr << "No CUDA devices found\n";
    return 1;
  }

  CUdevice device;
  cuDeviceGet(&device, 0);

  CUcontext context;
  cuCtxCreate(&context, 0, device);

  CUmodule module;
  for (auto p : actual_paths) {
    if (p.find(".fatbin") == std::string::npos) {
      std::cout << p << " Doesn't have the .fatbin suffix" << p << std::endl;
      return 1;
    }
    std::cout << "trying to load fatbin: " << p << std::endl;
    CUresult result = cuModuleLoad(&module, p.c_str());
    std::cout << "module pointer: " << module << '\n';
    if (result != CUDA_SUCCESS || module == nullptr) {
      std::cerr << "Failed to load the embedded fatbin with error: "
                << static_cast<unsigned int>(result) << '\n';
      return 1;
    }
  }

  return 0;
}
