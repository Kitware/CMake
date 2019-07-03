#include <iostream>
#include <string>
#include <vector>

#include "cmCTestHardwareSpec.h"

struct ExpectedSpec
{
  std::string Path;
  bool ParseResult;
  cmCTestHardwareSpec Expected;
};

static const std::vector<ExpectedSpec> expectedHardwareSpecs = {
  /* clang-format off */
  {"spec1.json", true, {{{
    {"gpus", {
      {"2", 4},
      {"e", 1},
    }},
    {"threads", {
    }},
  }}}},
  {"spec2.json", true, {{{
  }}}},
  {"spec3.json", false, {{{}}}},
  {"spec4.json", false, {{{}}}},
  {"spec5.json", false, {{{}}}},
  {"spec6.json", false, {{{}}}},
  {"spec7.json", false, {{{}}}},
  {"spec8.json", false, {{{}}}},
  {"spec9.json", false, {{{}}}},
  {"spec10.json", false, {{{}}}},
  {"spec11.json", false, {{{}}}},
  {"spec12.json", false, {{{}}}},
  {"spec13.json", false, {{{}}}},
  {"spec14.json", true, {{{}}}},
  {"spec15.json", true, {{{}}}},
  {"spec16.json", true, {{{}}}},
  {"spec17.json", false, {{{}}}},
  {"spec18.json", false, {{{}}}},
  {"noexist.json", false, {{{}}}},
  /* clang-format on */
};

static bool testSpec(const std::string& path, bool expectedResult,
                     const cmCTestHardwareSpec& expected)
{
  cmCTestHardwareSpec actual;
  bool result = actual.ReadFromJSONFile(path);
  if (result != expectedResult) {
    std::cout << "ReadFromJSONFile(\"" << path << "\") returned " << result
              << ", should be " << expectedResult << std::endl;
    return false;
  }

  if (result && actual != expected) {
    std::cout << "ReadFromJSONFile(\"" << path
              << "\") did not give expected spec" << std::endl;
    return false;
  }

  return true;
}

int testCTestHardwareSpec(int argc, char** const argv)
{
  if (argc < 2) {
    std::cout << "Invalid arguments.\n";
    return -1;
  }

  int retval = 0;
  for (auto const& spec : expectedHardwareSpecs) {
    std::string path = argv[1];
    path += "/testCTestHardwareSpec_data/";
    path += spec.Path;
    if (!testSpec(path, spec.ParseResult, spec.Expected)) {
      retval = -1;
    }
  }

  return retval;
}
