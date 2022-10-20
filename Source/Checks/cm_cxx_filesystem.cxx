
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

  // If std::string is copy-on-write, the std::filesystem::path
  // implementation may accidentally trigger a reallocation and compute
  // an offset between two allocations, leading to undefined behavior.
#if defined(__GLIBCXX__) &&                                                   \
  (!defined(_GLIBCXX_USE_CXX11_ABI) || !_GLIBCXX_USE_CXX11_ABI)
  std::string p5s1 = "/path";
  std::string p5s2 = std::move(p5s1);
  std::filesystem::path p5 = std::string(p5s2);
  p5.remove_filename();
#endif

  return 0;
}
