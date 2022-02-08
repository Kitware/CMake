#include <iostream>
#include <string>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(*a)

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produces output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  bool is_filtered =
    argc > 2 && std::string(argv[2]).find("--gtest_filter=") == 0;
  bool is_basic_only =
    is_filtered && std::string(argv[2]).find("basic*") != std::string::npos;
  bool is_typed_only =
    is_filtered && std::string(argv[2]).find("typed*") != std::string::npos;

  if (argc > 1 && std::string(argv[1]) == "--gtest_list_tests") {
    if (!is_typed_only) {
      const char* basic_suite_names[] = { "basic.", "ns.basic." };
      for (size_t i = 0; i < ARRAY_SIZE(basic_suite_names); i++) {
        std::cout << basic_suite_names[i] << std::endl;
        std::cout << "  case_foo" << std::endl;
        std::cout << "  case_bar" << std::endl;
        std::cout << "  DISABLED_disabled_case" << std::endl;
        std::cout << "  DISABLEDnot_really_case" << std::endl;
      }
    }
    if (!is_basic_only && !is_typed_only) {
      std::cout << "DISABLED_disabled." << std::endl;
      std::cout << "  case" << std::endl;
      std::cout << "DISABLEDnotreally." << std::endl;
      std::cout << "  case" << std::endl;
    }
    if (!is_basic_only) {
      const char* typed_suite_names[] = { "typed", "ns.typed" };
      for (size_t i = 0; i < ARRAY_SIZE(typed_suite_names); i++) {
        std::cout << typed_suite_names[i] << "/0.  # TypeParam = short\n";
        std::cout << "  case" << std::endl;
        std::cout << typed_suite_names[i] << "/1.  # TypeParam = float\n";
        std::cout << "  case" << std::endl;
        std::cout << typed_suite_names[i] << "/42.  # TypeParam = char\n";
        std::cout << "  case" << std::endl;
      }
    }
    if (!is_basic_only && !is_typed_only) {
      const char* value_suite_names[] = { "value", "ns.value" };
      for (size_t i = 0; i < ARRAY_SIZE(value_suite_names); i++) {
        std::cout << value_suite_names[i] << "/test." << std::endl;
        std::cout << "  case/0  # GetParam() = 1" << std::endl;
        std::cout << "  case/1  # GetParam() = \"foo\"" << std::endl;
      }
      const char* param_suite_names[] = { "param", "ns.param" };
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
      }
    }
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
