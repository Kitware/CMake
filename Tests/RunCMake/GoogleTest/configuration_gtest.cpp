#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  if (argc > 1 && std::string(argv[1]) == "--gtest_list_tests") {
    std::cout << "configuration." << std::endl;
#ifdef DEBUG
    std::cout << "  DISABLED_case_release" << std::endl;
    std::cout << "  case_debug" << std::endl;
#else
    std::cout << "  case_release" << std::endl;
    std::cout << "  DISABLED_case_debug" << std::endl;
#endif
    return 0;
  }

  return 1;
}
