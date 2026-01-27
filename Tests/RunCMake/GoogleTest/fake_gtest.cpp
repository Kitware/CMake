#define _CRT_SECURE_NO_WARNINGS // work around 'getenv' deprecation on WIN32

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#if defined(_MSC_VER) && _MSC_VER < 1310
#  define GETENV ::getenv
#else
#  define GETENV std::getenv
#endif

#define ARRAY_SIZE(a) sizeof(a) / sizeof(*a)

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  bool is_filtered =
    argc > 3 && std::string(argv[3]).find("--gtest_filter=") == 0;
  bool add_basic_tests =
    !is_filtered || (std::string(argv[3]).find("basic*") != std::string::npos);
  bool add_typed_tests =
    !is_filtered || (std::string(argv[3]).find("typed*") != std::string::npos);
  bool add_value_tests =
    !is_filtered || (std::string(argv[3]).find("value*") != std::string::npos);
  bool add_dynamic_tests = !is_filtered ||
    (std::string(argv[3]).find("dynamic*") != std::string::npos);

  if (argc > 2 && std::string(argv[1]) == "--gtest_list_tests" &&
      std::string(argv[2]).find("--gtest_output=json:") != std::string::npos) {
    std::ofstream ostrm;
    if (!GETENV("NO_GTEST_JSON_OUTPUT")) {
      std::string output_param(argv[2]);
      std::string::size_type split = output_param.find(":");
      std::string filepath = output_param.substr(split + 1);
      // The full file path is passed
      ostrm.open(filepath.c_str(), std::ios::out | std::ios::binary);
      if (!ostrm) {
        std::cerr << "Failed to create file: " << filepath.c_str()
                  << std::endl;
        return 1;
      }
    } else {
      std::cout << "WARNING: An old-style warning." << std::endl;
      std::cout << "[ WARNING ] A new-style warning." << std::endl;
      std::cout << "[  ERROR  ] A new-style error." << std::endl;
    }

    int tests = 0;
    ostrm << "{\n"
             "    \"name\": \"AllTests\",\n"
             "    \"testsuites\": [";

    if (add_basic_tests) {
      char const* basic_suite_names[] = { "basic", "ns.basic" };
      for (size_t i = 0; i < ARRAY_SIZE(basic_suite_names); i++) {
        std::cout << basic_suite_names[i] << '.' << std::endl;
        std::cout << "  case_foo" << std::endl;
        std::cout << "  case_bar" << std::endl;
        std::cout << "  DISABLED_disabled_case" << std::endl;
        std::cout << "  DISABLEDnot_really_case" << std::endl;

        if (tests)
          ostrm << ",";
        ostrm << "\n"
                 "        {\n"
                 "            \"name\": \""
              << basic_suite_names[i]
              << "\",\n"
                 "            \"tests\": 4,\n"
                 "            \"testsuite\": [\n"
                 "                { \"name\": \"case_foo\", \"file\": "
                 "\"file1.cpp\", \"line\": 1 },\n"
                 "                { \"name\": \"case_bar\", \"file\": "
                 "\"file with spaces.cpp\", \"line\": 2 },\n"
                 "                { \"name\": \"DISABLED_disabled_case\", "
                 "\"file\": \"file1.cpp\", \"line\": 3 },\n"
                 "                { \"name\": \"DISABLEDnot_really_case\", "
                 "\"file\": \"file1.cpp\", \"line\": 4 }\n"
                 "            ]\n"
                 "        }";
        tests += 4;
      }
    }
    if (!is_filtered) {
      std::cout << "DISABLED_disabled." << std::endl;
      std::cout << "  case" << std::endl;
      std::cout << "DISABLEDnotreally." << std::endl;
      std::cout << "  case" << std::endl;

      if (tests)
        ostrm << ",";
      ostrm << "\n"
               "        {\n"
               "            \"name\": \"DISABLED_disabled\",\n"
               "            \"tests\": 1,\n"
               "            \"testsuite\": [\n"
               "                { \"name\": \"case\", \"file\": "
               "\"file2.cpp\", \"line\": 1 }\n"
               "            ]\n"
               "        },\n"
               "        {\n"
               "            \"name\": \"DISABLEDnotreally\",\n"
               "            \"tests\": 1,\n"
               "            \"testsuite\": [\n"
               "                { \"name\": \"case\", \"file\": "
               "\"file2.cpp\", \"line\": 2 }\n"
               "            ]\n"
               "        }";
      tests += 2;
    }
    if (add_typed_tests) {
      char const* typed_suite_names[] = { "typed", "ns.typed",
                                          "prefix/typed" };
      for (size_t i = 0; i < ARRAY_SIZE(typed_suite_names); i++) {
        std::cout << typed_suite_names[i] << "/0.  # TypeParam = short\n";
        std::cout << "  case" << std::endl;
        std::cout << typed_suite_names[i] << "/1.  # TypeParam = float\n";
        std::cout << "  case" << std::endl;
        std::cout << typed_suite_names[i] << "/42.  # TypeParam = char\n";
        std::cout << "  case" << std::endl;
        std::cout << typed_suite_names[i] << "/named.  # TypeParam = int\n";
        std::cout << "  case" << std::endl;

        if (tests)
          ostrm << ",";
        ostrm << "\n"
                 "        {\n"
                 "            \"name\": \""
              << typed_suite_names[i]
              << "/0\",\n"
                 "            \"tests\": 1, \"testsuite\": [ { \"name\": "
                 "\"case\", \"type_param\": \"short\", \"file\": "
                 "\"file3.cpp\", \"line\": 1 } ]\n"
                 "        },\n"
                 "        {\n"
                 "            \"name\": \""
              << typed_suite_names[i]
              << "/1\",\n"
                 "            \"tests\": 1, \"testsuite\": [ { \"name\": "
                 "\"case\", \"type_param\": \"float\", \"file\": "
                 "\"file3.cpp\", \"line\": 2 } ]\n"
                 "        },\n"
                 "        {\n"
                 "            \"name\": \""
              << typed_suite_names[i]
              << "/42\",\n"
                 "            \"tests\": 1, \"testsuite\": [ { \"name\": "
                 "\"case\", \"type_param\": \"char\", \"file\": "
                 "\"file3.cpp\", \"line\": 3 } ]\n"
                 "        },\n"
                 "        {\n"
                 "            \"name\": \""
              << typed_suite_names[i]
              << "/named\",\n"
                 "            \"tests\": 1, \"testsuite\": [ { \"name\": "
                 "\"case\", \"type_param\": \"int\", \"file\": \"file3.cpp\", "
                 "\"line\": 4 } ]\n"
                 "        }";
        tests += 4;
      }
    }
    if (add_value_tests) {
      char const* value_suite_names[] = { "value", "ns.value",
                                          "prefix/value" };
      for (size_t i = 0; i < ARRAY_SIZE(value_suite_names); i++) {
        std::cout << value_suite_names[i] << "/test." << std::endl;
        std::cout << "  case/0  # GetParam() = 1" << std::endl;
        std::cout << "  case/1  # GetParam() = \"foo\"" << std::endl;
        std::cout << "  case/named  # GetParam() = 'c'" << std::endl;

        if (tests)
          ostrm << ",";
        ostrm
          << "\n"
             "        {\n"
             "            \"name\": \""
          << value_suite_names[i] << "/test"
          << "\",\n"
             "            \"tests\": 3,\n"
             "            \"testsuite\": [\n"
             "                { \"name\": \"case/0\", \"value_param\": \"1\", "
             "\"file\": \"file4.cpp\", \"line\": 1 },\n"
             "                { \"name\": \"case/1\", \"value_param\": "
             "\"\\\"foo\\\"\", \"file\": \"file4.cpp\", \"line\": 2 },\n"
             "                { \"name\": \"case/named\", \"value_param\": "
             "\"'c'\", \"file\": \"file4.cpp\", \"line\": 3 }\n"
             "            ]\n"
             "        }";
        tests += 3;
      }
      char const* param_suite_names[] = { "param", "ns.param",
                                          "prefix/param" };
      for (size_t j = 0; j < ARRAY_SIZE(param_suite_names); j++) {
        std::cout << param_suite_names[j] << "/special." << std::endl;
        std::cout << "  case/0  # GetParam() = \"semicolon;\"" << std::endl;
        std::cout << "  case/1  # GetParam() = \"backslash\\\"" << std::endl;
        std::cout << "  case/2  # GetParam() = \"${var}\"" << std::endl;
        std::cout << "  case/3  # GetParam() = '['" << std::endl;
        std::cout << "  case/4  # GetParam() = \"]]=]\"" << std::endl;
        std::cout << "  case/5  # GetParam() = \"__osbtext\"" << std::endl;
        std::cout << "  case/6  # GetParam() = \"__csb___text\"" << std::endl;
        std::cout << "  case/7  # GetParam() = \"S o m  e   \"" << std::endl;

        ostrm
          << ",\n"
             "        {\n"
             "            \"name\": \""
          << param_suite_names[j] << "/special"
          << "\",\n"
             "            \"tests\": 8,\n"
             "            \"testsuite\": [\n"
             "                { \"name\": \"case/0\", \"value_param\": "
             "\"\\\"semicolon;\\\"\", \"file\": \"file4.cpp\", \"line\": 1 "
             "},\n"
             "                { \"name\": \"case/1\", \"value_param\": "
             "\"\\\"backslash\\\\\\\"\", \"file\": \"file4.cpp\", \"line\": 2 "
             "},\n"
             "                { \"name\": \"case/2\", \"value_param\": "
             "\"\\\"${var}\\\"\", \"file\": \"file4.cpp\", \"line\": 3 },\n"
             "                { \"name\": \"case/3\", \"value_param\": "
             "\"'['\", \"file\": \"file4.cpp\", \"line\": 4 },\n"
             "                { \"name\": \"case/4\", \"value_param\": "
             "\"\\\"]]=]\\\"\", \"file\": \"file4.cpp\", \"line\": 5 },\n"
             "                { \"name\": \"case/5\", \"value_param\": "
             "\"\\\"__osbtext\\\"\", \"file\": \"file4.cpp\", \"line\": 5 },\n"
             "                { \"name\": \"case/6\", \"value_param\": "
             "\"\\\"__csb___text\\\"\", \"file\": \"file4.cpp\", \"line\": 5 "
             "},\n"
             "                { \"name\": \"case/7\", \"value_param\": "
             "\"\\\"S o m  e   \\\"\", \"file\": \"file4.cpp\", \"line\": 6 "
             "}\n"
             "            ]\n"
             "        }";
        tests += 8;
      }
    }
    if (add_value_tests || add_typed_tests || add_dynamic_tests) {
      char const* both_suite_names[] = { "both_suite", "both/suite",
                                         "ns.both/suite",
                                         "prefix/both/suite" };
      for (size_t k = 0; k < ARRAY_SIZE(both_suite_names); k++) {
        std::cout << both_suite_names[k] << ".  # TypeParam = TYPE"
                  << std::endl;
        std::cout << "  test  # GetParam() = VALUE" << std::endl;
        std::cout << "  case/test  # GetParam() = VALUE" << std::endl;

        if (tests)
          ostrm << ",";
        ostrm << "\n"
                 "          {\n"
                 "              \"name\": \""
              << both_suite_names[k]
              << "\",\n"
                 "              \"tests\": 1,\n"
                 "              \"testsuite\": [\n"
                 "                  { \"name\": \"test\", \"type_param\": "
                 "\"TYPE\", \"value_param\": \"VALUE\", \"file\": "
                 "\"file5.cpp\", \"line\": 1 },\n"
                 "                  { \"name\": \"case/test\", "
                 "\"type_param\": \"TYPE\", \"value_param\": \"VALUE\", "
                 "\"file\": \"file5.cpp\", \"line\": 2 }\n"
                 "              ]\n"
                 "          }";
        tests += 2;
      }
    }
    ostrm << "\n"
             "    ],\n"
             "    \"tests\": "
          << tests << "\n}";
    return 0;
  }

  if (argc > 5) {
    // Simple test of EXTRA_ARGS
    if (std::string(argv[3]) == "how" && std::string(argv[4]) == "now" &&
        std::string(argv[5]) == "\"brown\" cow") {
      return 0;
    }
  }

  // Print arguments for debugging, if we didn't get the expected arguments
  for (int i = 1; i < argc; ++i) {
    std::cerr << "arg[" << i << "]: '" << argv[i] << "'\n";
  }

  return 1;
}
