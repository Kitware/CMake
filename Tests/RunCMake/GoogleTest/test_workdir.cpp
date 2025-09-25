#include <fstream>
#include <iostream>
#include <string>

/* Having this as a comment lets gtest_add_tests() recognize the tests we fake
   here without requiring googletest
TEST_F( WorkDirWithSpaces, test1 )
{
}
TEST_F( WorkDirWithSpaces, test2 )
{
}
*/

int main(int argc, char** argv)
{
  // Fake gtest behavior so we don't actually require gtest.
  // Both listing tests and test execution try to read a file from
  // the current directory. If we are not handling spaces in the
  // working directory correctly, the files we expect won't exist.

  if (argc > 1 && std::string(argv[1]) == "--gtest_list_tests") {
    std::ifstream inFile("test_list_output.txt");
    if (!inFile) {
      std::cout << "ERROR: Failed to open test_list_output.txt" << std::endl;
      return 1;
    }
    std::cout << inFile.rdbuf();
    return 0;
  }

  std::ifstream inFile("test_output.txt");
  if (!inFile) {
    std::cout << "ERROR: Failed to open test_output.txt" << std::endl;
    return 1;
  }
  std::cout << "Test output read from file to follow:" << std::endl;
  std::cout << inFile.rdbuf();
  return 0;
}
