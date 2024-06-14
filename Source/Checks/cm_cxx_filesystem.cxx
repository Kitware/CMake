
#include <filesystem>
#if defined(__GLIBCXX__)
#  include <string_view>
#endif

int main()
{
  std::filesystem::path p0(L"/a/b/c");

  std::filesystem::path p1("/a/b/c");
  std::filesystem::path p2("/a/b//c");
  if (p1 != p2.lexically_normal()) {
    return 1;
  }

#if defined(_WIN32)
  // "//host/" is not preserved in some environments like GNU under MinGW.
  std::filesystem::path p3("//host/a/b/../c");
  if (p3.lexically_normal().generic_string() != "//host/a/c") {
    return 1;
  }

  std::filesystem::path p4("c://a/.///b/../");
  if (p4.lexically_normal().generic_string() != "c:/a/") {
    return 1;
  }

  std::filesystem::path b1("C:\\path\\y\\..\\");
  if (std::filesystem::weakly_canonical("\\\\?\\C:\\path\\x\\..") !=
      b1.lexically_normal()) {
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

#if defined(__GLIBCXX__)
  // RH gcc-toolset-10 has a strange bug: it selects, in some circumstances,
  // the wrong constructor which generate error in template instantiation.
  class my_string_view : std::string_view
  {
  public:
    my_string_view(const char* p)
      : std::string_view(p)
    {
    }
  };
  class my_path
  {
  public:
    my_path(std::filesystem::path path) {}

    my_path(my_string_view path) {}
  };

  my_path p{ my_string_view{ "abc" } };
  // here is the bug: the constructor taking std::filesystem::path as argument
  // is selected, so the compiler try to build a std::filesystem::path instance
  // from the my_string_view argument and fails to do so.
#endif

  return 0;
}
