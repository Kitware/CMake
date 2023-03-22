#include <iostream>
#include <string>
#include <vector>

#include "cmCTestResourceSpec.h"
#include "cmJSONState.h"

struct ExpectedSpec
{
  std::string Path;
  bool ParseResult;
  cmCTestResourceSpec Expected;
};

static const std::vector<ExpectedSpec> expectedResourceSpecs = {
  { "spec1.json",
    true,
    { { {
        { "gpus",
          {
            { "2", 4 },
            { "e", 1 },
          } },
        { "threads", {} },
      } },
      cmJSONState() } },
  { "spec2.json", true, {} },
  { "spec3.json", false, {} },
  { "spec4.json", false, {} },
  { "spec5.json", false, {} },
  { "spec6.json", false, {} },
  { "spec7.json", false, {} },
  { "spec8.json", false, {} },
  { "spec9.json", false, {} },
  { "spec10.json", false, {} },
  { "spec11.json", false, {} },
  { "spec12.json", false, {} },
  { "spec13.json", false, {} },
  { "spec14.json", true, {} },
  { "spec15.json", true, {} },
  { "spec16.json", true, {} },
  { "spec17.json", false, {} },
  { "spec18.json", false, {} },
  { "spec19.json", false, {} },
  { "spec20.json", true, {} },
  { "spec21.json", false, {} },
  { "spec22.json", false, {} },
  { "spec23.json", false, {} },
  { "spec24.json", false, {} },
  { "spec25.json", false, {} },
  { "spec26.json", false, {} },
  { "spec27.json", false, {} },
  { "spec28.json", false, {} },
  { "spec29.json", false, {} },
  { "spec30.json", false, {} },
  { "spec31.json", false, {} },
  { "spec32.json", false, {} },
  { "spec33.json", false, {} },
  { "spec34.json", false, {} },
  { "spec35.json", false, {} },
  { "spec36.json", false, {} },
  { "noexist.json", false, {} },
};

static bool testSpec(const std::string& path, bool expectedResult,
                     const cmCTestResourceSpec& expected)
{
  cmCTestResourceSpec actual;
  auto result = actual.ReadFromJSONFile(path);
  if (result != expectedResult) {
    std::cout << "ReadFromJSONFile(\"" << path << "\") failed \"" << std::endl;
    return false;
  }

  if (actual != expected) {
    std::cout << "ReadFromJSONFile(\"" << path
              << "\") did not give expected spec" << std::endl;
    return false;
  }

  return true;
}

int testCTestResourceSpec(int argc, char** const argv)
{
  if (argc < 2) {
    std::cout << "Invalid arguments.\n";
    return -1;
  }

  int retval = 0;
  for (auto const& spec : expectedResourceSpecs) {
    std::string path = argv[1];
    path += "/testCTestResourceSpec_data/";
    path += spec.Path;
    if (!testSpec(path, spec.ParseResult, spec.Expected)) {
      retval = -1;
    }
  }

  return retval;
}
