#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  if (argc > 2 && std::string(argv[1]) == "--gtest_list_tests" &&
      std::string(argv[2]).find("--gtest_output=json:") != std::string::npos) {
    std::cout << "flush_script_test.\n";

    std::string output_param(argv[2]);
    std::string::size_type split = output_param.find(":");
    std::string filepath = output_param.substr(split + 1);
    // The full file path is passed
    std::ofstream ostrm(filepath.c_str(), std::ios::binary);
    if (!ostrm) {
      std::cerr << "Failed to create file: " << filepath.c_str() << std::endl;
      return 1;
    }

    size_t const flushThreshold = 50000;
    size_t const flushAfter = 4;
    size_t const testCaseNum = 3 * flushAfter;
    ostrm << "{\n"
             "    \"tests\": 4,\n"
             "    \"name\": \"AllTests\",\n"
             "    \"testsuites\": [\n"
             "        {\n"
             "            \"name\": \"flush_script_test\",\n"
             "            \"tests\": 12,\n"
             "            \"testsuite\": [";

    std::string testName(flushThreshold / flushAfter, 'T');
    for (size_t i = 1; i <= testCaseNum; ++i) {
      std::cout << "  t" << i << testName.c_str() << "\n";
      if (i != 1)
        ostrm << ",";
      ostrm << "\n"
               "                {\n"
               "                  \"name\": \"t"
            << i << testName.c_str()
            << "\",\n"
               "                  \"file\": \"file2.cpp\",\n"
               "                  \"line\": 47\n"
               "                }";
    }
    ostrm << "\n"
             "            ]\n"
             "        }\n"
             "    ]\n"
             "}";
  }

  return 0;
}
