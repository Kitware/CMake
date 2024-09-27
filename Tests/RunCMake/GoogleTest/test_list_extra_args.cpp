#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: This test doesn't actually depend on Google Test as such;
  // it only requires that we produce output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.

  // Simple test of DISCOVERY_EXTRA_ARGS
  if (argc > 4 && std::string(argv[1]) == "--gtest_list_tests" &&
      std::string(argv[2]) == "how now" && std::string(argv[3]) == "" &&
      std::string(argv[4]) == "\"brown\" cow") {
    std::cout << "test_list_test/test.\n";
    std::cout << "  case/0  # GetParam() = 'one'\n";
    std::cout << "  case/1  # GetParam() = 'two'\n";
    std::cout << "  case/2  # GetParam() = 'three'\n";
    std::cout << "  case/3  # GetParam() = 'four'\n";
  }
  return 0;
}
