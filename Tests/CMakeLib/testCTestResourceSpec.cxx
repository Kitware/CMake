#include <iostream>
#include <string>
#include <vector>

#include "cmCTestResourceSpec.h"

struct ExpectedSpec
{
  std::string Path;
  cmCTestResourceSpec::ReadFileResult ParseResult;
  cmCTestResourceSpec Expected;
};

static const std::vector<ExpectedSpec> expectedResourceSpecs = {
  { "spec1.json",
    cmCTestResourceSpec::ReadFileResult::READ_OK,
    { { {
      { "gpus",
        {
          { "2", 4 },
          { "e", 1 },
        } },
      { "threads", {} },
    } } } },
  { "spec2.json", cmCTestResourceSpec::ReadFileResult::READ_OK, {} },
  { "spec3.json",
    cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC,
    {} },
  { "spec4.json",
    cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC,
    {} },
  { "spec5.json",
    cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC,
    {} },
  { "spec6.json",
    cmCTestResourceSpec::ReadFileResult::INVALID_SOCKET_SPEC,
    {} },
  { "spec7.json",
    cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE_TYPE,
    {} },
  { "spec8.json", cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE, {} },
  { "spec9.json", cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE, {} },
  { "spec10.json", cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE, {} },
  { "spec11.json", cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE, {} },
  { "spec12.json", cmCTestResourceSpec::ReadFileResult::INVALID_ROOT, {} },
  { "spec13.json", cmCTestResourceSpec::ReadFileResult::JSON_PARSE_ERROR, {} },
  { "spec14.json", cmCTestResourceSpec::ReadFileResult::READ_OK, {} },
  { "spec15.json", cmCTestResourceSpec::ReadFileResult::READ_OK, {} },
  { "spec16.json", cmCTestResourceSpec::ReadFileResult::READ_OK, {} },
  { "spec17.json", cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE, {} },
  { "spec18.json", cmCTestResourceSpec::ReadFileResult::INVALID_RESOURCE, {} },
  { "spec19.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec20.json", cmCTestResourceSpec::ReadFileResult::READ_OK, {} },
  { "spec21.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec22.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec23.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec24.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec25.json",
    cmCTestResourceSpec::ReadFileResult::UNSUPPORTED_VERSION,
    {} },
  { "spec26.json",
    cmCTestResourceSpec::ReadFileResult::UNSUPPORTED_VERSION,
    {} },
  { "spec27.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec28.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec29.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec30.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec31.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec32.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec33.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec34.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec35.json", cmCTestResourceSpec::ReadFileResult::INVALID_VERSION, {} },
  { "spec36.json", cmCTestResourceSpec::ReadFileResult::NO_VERSION, {} },
  { "noexist.json", cmCTestResourceSpec::ReadFileResult::FILE_NOT_FOUND, {} },
};

static bool testSpec(const std::string& path,
                     cmCTestResourceSpec::ReadFileResult expectedResult,
                     const cmCTestResourceSpec& expected)
{
  cmCTestResourceSpec actual;
  auto result = actual.ReadFromJSONFile(path);
  if (result != expectedResult) {
    std::cout << "ReadFromJSONFile(\"" << path << "\") returned \""
              << cmCTestResourceSpec::ResultToString(result)
              << "\", should be \""
              << cmCTestResourceSpec::ResultToString(expectedResult) << "\""
              << std::endl;
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
