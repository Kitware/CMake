#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  if (argc >= 2 && std::string(argv[1]) == "--gtest_list_tests") {
    bool saw_json = false;
    bool saw_basic_filter = false;
    for (int i = 2; i < argc; ++i) {
      std::string const arg = argv[i];
      if (arg.find("--gtest_output=json:") == 0) {
        saw_json = true;
      } else if (arg == "--gtest_filter=basic*") {
        saw_basic_filter = true;
      } else {
        return 1;
      }
    }

    if (saw_json && argc >= 3) {
      for (int i = 2; i < argc; ++i) {
        std::string const arg = argv[i];
        if (arg.find("--gtest_output=json:") == 0) {
          std::string const json_path = arg.substr(20);
          std::ofstream json(json_path.c_str());
          json << "{\n"
                  "  \"name\": \"AllTests\",\n"
                  "  \"testsuites\": [\n"
                  "    {\n"
                  "      \"name\": \"basic\",\n"
                  "      \"tests\": 2,\n"
                  "      \"testsuite\": [\n"
                  "        { \"name\": \"case_foo\", \"file\": \"file1.cpp\", "
                  "\"line\": 1 },\n"
                  "        { \"name\": \"case_bar\", \"file\": \"file1.cpp\", "
                  "\"line\": 2 }\n"
                  "      ]\n"
                  "    }\n"
                  "  ]\n"
                  "}\n";
          break;
        }
      }
    }

    if (!saw_basic_filter && argc >= 3) {
      return 1;
    }

    std::cout << "basic.\n";
    std::cout << "  case_foo\n";
    std::cout << "  case_bar\n";
    return 0;
  }

  return 0;
}
