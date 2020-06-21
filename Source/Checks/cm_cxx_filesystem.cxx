
#include <filesystem>

int main()
{
  std::filesystem::path p1("/a/b/c");
  std::filesystem::path p2("/a/b/c");

  return p1 == p2 ? 0 : 1;
}
