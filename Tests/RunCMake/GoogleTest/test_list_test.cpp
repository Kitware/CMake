#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  if (argc > 1 && std::string(argv[1]) == "--gtest_list_tests") {
    std::cout << "test_list_test/test.\n";
    std::cout << "  case/0  # GetParam() = \"semicolon;\"\n";
    std::cout << "  case/1  # GetParam() = 'osb['\n";
    std::cout << "  case/2  # GetParam() = 'csb]'\n";
    std::cout << "  case/3  # GetParam() = 'S p a c e s'\n";
  }
  return 0;
}
