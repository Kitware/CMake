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

  if (argc > 2 && std::string(argv[1]) == "--gtest_list_tests" &&
      std::string(argv[2]).find("--gtest_output=json:") != std::string::npos) {
    std::ifstream inTestListFile("test_list_output.txt");
    if (!inTestListFile) {
      std::cout << "ERROR: Failed to open test_list_output.txt" << std::endl;
      return 1;
    }
    std::cout << inTestListFile.rdbuf();

    std::ifstream inJsonFile("test_list_output.json");
    if (!inJsonFile) {
      std::cout << "ERROR: Failed to open test_list_output.json" << std::endl;
      return 1;
    }
    std::string output_param(argv[2]);
    std::string::size_type split = output_param.find(":");
    std::string filepath = output_param.substr(split + 1);
    // The full file path is passed
    std::ofstream ostrm(filepath.c_str(), std::ios::binary);
    if (!ostrm) {
      std::cerr << "Failed to create file: " << filepath.c_str() << std::endl;
      return 1;
    }
    ostrm << inJsonFile.rdbuf();
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
