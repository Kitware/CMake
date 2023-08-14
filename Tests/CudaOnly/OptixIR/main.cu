#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <cuda.h>

#define GENERATED_HEADER(x) GENERATED_HEADER1(x)
#define GENERATED_HEADER1(x) <x>

static std::string input_paths = { OPTIX_FILE_PATHS };

int main()
{
  if (input_paths == "NO_OPTIX_SUPPORT") {
    return 0;
  }

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

  if (actual_paths.empty()) {
    std::cerr << "Failed to parse OPTIX_FILE_PATHS" << std::endl;
    return 1;
  }

  const std::uint32_t optix_magic_value = 0x7f4e43ed;
  for (auto p : actual_paths) {
    if (p.find(".optixir") == std::string::npos) {
      std::cout << p << " Doesn't have the .optixir suffix" << p << std::endl;
      return 1;
    }
    std::ifstream input(p, std::ios::binary);
    std::uint32_t value;
    input.read(reinterpret_cast<char*>(&value), sizeof(value));
    if (value != optix_magic_value) {
      std::cerr << p << " Doesn't look like an optix-ir file" << std::endl;
      return 1;
    }
  }

  return 0;
}
