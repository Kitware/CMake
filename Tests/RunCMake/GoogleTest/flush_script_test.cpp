#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  if (argc > 1 && std::string(argv[1]) == "--gtest_list_tests") {
    std::cout << "flush_script_test.\n";
    size_t const flushThreshold = 50000;
    size_t const flushAfter = 4;
    size_t const testCaseNum = 3 * flushAfter;
    std::string testName(flushThreshold / flushAfter, 'T');
    for (size_t i = 1; i <= testCaseNum; ++i)
      std::cout << "  t" << i << testName.c_str() << "\n";
  }
  return 0;
}
