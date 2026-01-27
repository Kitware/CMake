#include <fstream>
#include <iostream>
#include <string>

/* Having this as comment lets gtest_add_tests recognizes the test we fake
   here without requiring googletest
TEST_F( skip_test, test1 )
{
}
*/

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  if (argc > 2 && std::string(argv[1]) == "--gtest_list_tests" &&
      std::string(argv[2]).find("--gtest_output=json:") != std::string::npos) {
    std::cout << "skip_test." << std::endl;
    std::cout << "  test1" << std::endl;

    std::string output_param(argv[2]);
    std::string::size_type split = output_param.find(":");
    std::string filepath = output_param.substr(split + 1);
    // The full file path is passed
    std::ofstream ostrm(filepath.c_str(), std::ios::binary);
    if (!ostrm) {
      std::cerr << "Failed to create file: " << filepath.c_str() << std::endl;
      return 1;
    }
    ostrm << "{\n"
             "    \"tests\": 1,\n"
             "    \"name\": \"AllTests\",\n"
             "    \"testsuites\": [\n"
             "        {\n"
             "            \"name\": \"skip_test\",\n"
             "            \"tests\": 1,\n"
             "            \"testsuite\": [\n"
             "                { \"name\": \"test1\", \"file\": \"file.cpp\", "
             "\"line\": 42 }\n"
             "            ]\n"
             "        }\n"
             "    ]\n"
             "}";

    return 0;
  }

  std::cout << "[  SKIPPED ] skip_test.test1" << std::endl;
  return 0;
}
