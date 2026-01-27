#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: This test doesn't actually depend on Google Test as such;
  // it only requires that we produce output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.

  // Simple test of DISCOVERY_EXTRA_ARGS
  if (argc > 5 && std::string(argv[1]) == "--gtest_list_tests" &&
      std::string(argv[2]).find("--gtest_output=json:") != std::string::npos &&
      std::string(argv[3]) == "how now" && std::string(argv[4]) == "" &&
      std::string(argv[5]) == "\"brown\" cow") {
    std::cout << "test_list_test/test.\n";
    std::cout << "  case/0  # GetParam() = 'one'\n";
    std::cout << "  case/1  # GetParam() = 'two'\n";
    std::cout << "  case/2  # GetParam() = 'three'\n";
    std::cout << "  case/3  # GetParam() = 'four'\n";

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
             "    \"tests\": 4,\n"
             "    \"name\": \"AllTests\",\n"
             "    \"testsuites\": [\n"
             "        {\n"
             "            \"name\": \"test_list_test/test\",\n"
             "            \"tests\": 4,\n"
             "            \"testsuite\": [\n"
             "                { \"name\": \"case/0\", \"file\": \"file.cpp\", "
             "\"line\": 42 },\n"
             "                { \"name\": \"case/1\", \"file\": \"file.cpp\", "
             "\"line\": 42 },\n"
             "                { \"name\": \"case/2\", \"file\": \"file.cpp\", "
             "\"line\": 42 },\n"
             "                { \"name\": \"case/3\", \"file\": \"file.cpp\", "
             "\"line\": 42 }\n"
             "            ]\n"
             "        }\n"
             "    ]\n"
             "}";
  }
  return 0;
}
