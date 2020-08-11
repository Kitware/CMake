#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  if (argc > 1 && std::string(argv[1]) == "--gtest_list_tests") {
    std::cout << "skip_test." << std::endl;
    std::cout << "  test1" << std::endl;
    return 0;
  }

  std::cout << "[  SKIPPED ] skip_test.test1" << std::endl;
  return 0;
}
