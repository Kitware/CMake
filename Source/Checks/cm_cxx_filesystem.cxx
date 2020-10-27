
#include <filesystem>

int main()
{
  std::filesystem::path p0(L"/a/b/c");

  std::filesystem::path p1("/a/b/c");
  std::filesystem::path p2("/a/b/c");
  if (p1 != p2) {
    return 1;
  }

#if defined(_WIN32)
  std::filesystem::path p3("//host/a/b/../c");
  if (p3.lexically_normal().generic_string() != "//host/a/c") {
    return 1;
  }

  std::filesystem::path p4("c://a/.///b/../");
  if (p4.lexically_normal().generic_string() != "c:/a/") {
    return 1;
  }
#endif

  return 0;
}
